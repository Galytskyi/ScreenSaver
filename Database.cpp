#include "Database.h"

#include <cassert>
#include <QMessageBox>
#include <QFile>

// -------------------------------------------------------------------------------------------------------------------

Database theDatabase;

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

SqlFieldBase::SqlFieldBase()
{
}

// -------------------------------------------------------------------------------------------------------------------

int SqlFieldBase::init(int objectType, int)
{
	switch(objectType)
	{
		case SQL_TABLE_DATABASE_INFO:

			append("ID",							QMetaType::Int);
			append("ObjectID",						QMetaType::Int);
			append("Name",							QMetaType::QString, 256);
			append("Version",						QMetaType::Int);

			break;

		case SQL_TABLE_PICTURE_INFO:

			append("ObjectID",						QMetaType::Int);
			append("PictureID",						QMetaType::Int);

            append("Hash",						QMetaType::Long);
            append("PacketTime",					QMetaType::QTime);

			append("ImageWidth",					QMetaType::Int);
			append("ImageHeight",					QMetaType::Int);

            append("CmpPercent",					QMetaType::Double);

			break;

		case SQL_TABLE_PICTURE:

			append("ObjectID",						QMetaType::Int);
			append("PictureID",						QMetaType::Int);

			append("PictureData",					QMetaType::QVariant);

			break;

		default:
			assert(0);
	}

	int fieldCount = count();
	assert(fieldCount);

	return fieldCount;
}

// -------------------------------------------------------------------------------------------------------------------


void SqlFieldBase::append(const QSqlField& field)
{
	QSqlRecord::append(field);
}

// -------------------------------------------------------------------------------------------------------------------

void SqlFieldBase::append(QString name, QMetaType::Type type, int length)
{
	if (name.isEmpty() == true)
	{
		return;
	}

	if (type == QMetaType::UnknownType)
	{
		return;
	}

	QSqlField field(name, QMetaType(type));

	if (type == QMetaType::Double)
	{
		field.setPrecision(9);
	}

	if (type == QMetaType::QString)
	{
		field.setLength(length);
	}

	append(field);
}

// -------------------------------------------------------------------------------------------------------------------

QString SqlFieldBase::extFieldName(int index)
{
	if (index < 0 || index >= count())
	{
		return QString();
	}

	QSqlField f = field(index);

	QString result;

	switch(f.metaType().id())
	{
		case QMetaType::Bool:		result = QString("%1 BOOLEAN").arg(f.name());								break;	// bool
		case QMetaType::Int:		result = QString("%1 INTEGER").arg(f.name());								break;	// qint32
		case QMetaType::Long:		result = QString("%1 BIGINT").arg(f.name());								break;	// qint64
		case QMetaType::Double:		result = QString("%1 DOUBLE PRECISION").arg(f.name());						break;	// double
		case QMetaType::QString:	result = QString("%1 VARCHAR(%2)").arg(f.name()).arg(f.length());			break;	// string
		case QMetaType::QVariant:	result = QString("%1 BYTEA").arg(f.name());									break;	// blob
		case QMetaType::QTime:		result = QString("%1 TIMESTAMP").arg(f.name());								break;	// date and time

		default:
			assert(0);
			result.clear();
	}

	return result;
}


// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

SqlObjectInfo::SqlObjectInfo()
{
}

// -------------------------------------------------------------------------------------------------------------------

bool SqlObjectInfo::init(int objectType)
{
	if (objectType < 0 || objectType >= SQL_TABLE_COUNT)
	{
		return false;
	}

	m_objectType = objectType;
	m_objectID = SqlObjectID[objectType];
	m_caption = SqlTableName[objectType];
	m_version = SqlTableVersion[objectType];

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

void SqlObjectInfo::clear()
{
	m_objectType = SQL_TABLE_UNKNONW;
	m_objectID = SQL_OBJECT_ID_UNKNONW;
	m_caption.clear();
	m_version = SQL_TABLE_VER_UNKNONW;
}

// -------------------------------------------------------------------------------------------------------------------

SqlObjectInfo& SqlObjectInfo::operator=(SqlObjectInfo& from)
{
	m_objectType = from.m_objectType;
	m_objectID = from.m_objectID;
	m_caption = from.m_caption;
	m_version = from.m_version;

	return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

SqlTable::SqlTable()
{
}

// -------------------------------------------------------------------------------------------------------------------

SqlTable::~SqlTable()
{
}

// -------------------------------------------------------------------------------------------------------------------

int SqlTable::recordCount() const
{
	if (isOpen() == false)
	{
		return 0;
	}

	int type = m_info.objectType();
	if (type < 0 || type >= SQL_TABLE_COUNT)
	{
		return 0;
	}

	QSqlQuery query(QString("SELECT count(*) FROM %1").arg(m_info.caption()));
	if (query.next() == false)
	{
		return 0;
	}

	return query.value(0).toInt();
}

// -------------------------------------------------------------------------------------------------------------------

int SqlTable::lastKey() const
{
	if (isOpen() == false)
	{
		return SQL_INVALID_KEY;
	}

	int type = m_info.objectType();
	if (type < 0 || type >= SQL_TABLE_COUNT)
	{
		return SQL_INVALID_KEY;
	}

	QSqlQuery query(QString("SELECT max(%1) FROM %2").arg(m_fieldBase.field(SQL_FIELD_KEY).name(), m_info.caption()));
	if (query.next() == false)
	{
		return SQL_INVALID_KEY;
	}

	return query.value(0).toInt();
}

// -------------------------------------------------------------------------------------------------------------------

bool SqlTable::init(int objectType, QSqlDatabase* pDatabase)
{
	if (objectType < 0 || objectType >= SQL_TABLE_COUNT)
	{
		return false;
	}

	if (pDatabase == nullptr)
	{
		return false;
	}

	if (m_info.init(objectType) == false)
	{
		return false;
	}

	m_pDatabase = pDatabase;

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

bool SqlTable::isExist() const
{
	if (m_pDatabase == nullptr)
	{
		return false;
	}

	if (m_pDatabase->isOpen() == false)
	{
		return false;
	}

	int type = m_info.objectType();
	if (type < 0 || type >= SQL_TABLE_COUNT)
	{
		return false;
	}

	bool tableIsExist = false;

	int existTableCount = static_cast<int>(m_pDatabase->tables().count());
	for(int et = 0; et < existTableCount; et++)
	{
		if (m_pDatabase->tables().at(et).compare(SqlTableName[type], Qt::CaseInsensitive) == 0)
		{
			tableIsExist = true;
			break;
		}
	}

	return tableIsExist;
}


// -------------------------------------------------------------------------------------------------------------------

bool SqlTable::open()
{
	if (isExist() == false)
	{
		return false;
	}

	if (m_fieldBase.init(m_info.objectType(), m_info.version()) == 0)
	{
		return false;
	}

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

void SqlTable::close()
{
	m_fieldBase.clear();
}

// -------------------------------------------------------------------------------------------------------------------

bool SqlTable::create()
{
	if (isExist() == true)
	{
		return false;
	}

	if (m_fieldBase.init(m_info.objectType(), m_info.version()) == 0)
	{
		return false;
	}

	QSqlQuery query;

	QString request = QString("CREATE TABLE if not exists %1 (").arg(m_info.caption());

	int filedCount = m_fieldBase.count();
	for(int field = 0; field < filedCount; field++)
	{
		request.append(m_fieldBase.extFieldName(field));

		if (field == SQL_FIELD_KEY)
		{
			request.append(" PRIMARY KEY NOT NULL");
		}

		if (field != filedCount - 1)
		{
			request.append(", ");
		}
	}

	request.append(");");

	return query.exec(request);
}

// -------------------------------------------------------------------------------------------------------------------

bool SqlTable::drop()
{
	QSqlQuery query;
	if (query.exec(QString("DROP TABLE %1").arg(m_info.caption())) == false)
	{
		return false;
	}

	close();

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

bool SqlTable::clear()
{
	if (isOpen() == false)
	{
		return false;
	}

	QSqlQuery query;

	if (query.exec("BEGIN TRANSACTION") == false)
	{
		return false;
	}

	if (query.exec(QString("DELETE FROM %1").arg(m_info.caption())) == false)
	{
		return false;
	}

	if (query.exec("COMMIT") == false)
	{
		return false;
	}

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

int SqlTable::read(void* pRecord, int* key, int keyCount)
{
	if (isOpen() == false)
	{
		return 0;
	}

	if (pRecord == nullptr)
	{
		return 0;
	}

	// create request
	//
	QString request = QString("SELECT * FROM %1").arg(m_info.caption());

	if (key != nullptr && keyCount != 0)
	{
		request.append(" WHERE ");
		QString keyFieldName = m_fieldBase.field(SQL_FIELD_KEY).name();

		for(int k = 0; k < keyCount; k++)
		{
			request.append(QString("%1=%2").arg(keyFieldName).arg(key[k]));

			if (k != keyCount - 1)
			{
				request.append(" OR ");
			}
		}
	}

	// exec select
	//
	QSqlQuery query;
	if (query.exec(request) == false)
	{
		return 0;
	}

	int field = 0;
	int objectID = SQL_OBJECT_ID_UNKNONW;
	int readedCount = 0;

	// read data
	//
	while (query.next() == true)
	{
		field = 0;

		// check unique ID of table or view, zero field always is ObjectID
		//
		objectID = query.value(field++).toInt();
		if (objectID != m_info.objectID())
		{
			continue;
		}

		// read field's data
		//
		switch(m_info.objectType())
		{
			case SQL_TABLE_DATABASE_INFO:
				{
					SqlObjectInfo* info = static_cast<SqlObjectInfo*> (pRecord) + readedCount;
					if (info == nullptr)
					{
						break;
					}

					info->setObjectID(query.value(field++).toInt());
					info->setCaption(query.value(field++).toString());
					info->setVersion(query.value(field++).toInt());
				}
				break;

			case SQL_TABLE_PICTURE_INFO:
				{
					DataPacket* packet = static_cast<DataPacket*> (pRecord) + readedCount;
					if (packet == nullptr)
					{
						break;
					}

					packet->setPacketID(query.value(field++).toInt());

                    packet->setHash(query.value(field++).toULongLong());
					packet->setPacketTime(query.value(field++).toDateTime());

					packet->setImageWidth(query.value(field++).toInt());
					packet->setImageHeight(query.value(field++).toInt());

                    packet->setCmpPercent(query.value(field++).toDouble());
				}
				break;

			case SQL_TABLE_PICTURE:
				{
					DataPacket* packet = static_cast<DataPacket*> (pRecord) + readedCount;
					if (packet == nullptr)
					{
						break;
					}

					packet->setPacketID(query.value(field++).toInt());

					packet->setImageData(query.value(field++).toByteArray());
				}
				break;

			default:
				assert(0);
		}

		readedCount ++;
	}

	return readedCount;
}

// -------------------------------------------------------------------------------------------------------------------

int SqlTable::write(void* pRecord, int count, int* key)
{
	if (isOpen() == false)
	{
		return 0;
	}

	if (pRecord == nullptr)
	{
		return 0;
	}

	if (count == 0)
	{
		return 0;
	}

	// create request
	//

	QString request;

	if (key == nullptr)
	{
		request = QString("INSERT INTO %1 (").arg(m_info.caption());

		int filedCount = m_fieldBase.count();
		for(int f = 0; f < filedCount; f++)
		{
			request.append(m_fieldBase.field(f).name());

			if (f != filedCount - 1)
			{
				request.append(", ");
			}
		}

		request.append(") VALUES (");

		for(int f = 0; f < filedCount; f++)
		{
			request.append("?");

			if (f != filedCount - 1)
			{
				request.append(", ");
			}
		}

		request.append(");");
	}
	else
	{
		request = QString("UPDATE %1 SET ").arg(m_info.caption());

		int filedCount = m_fieldBase.count();
		for(int f = 0; f < filedCount; f++)
		{
			request.append(QString("%1=?").arg(m_fieldBase.field(f).name()));

			if (f != filedCount - 1)
			{
				request.append(", ");
			}
		}

		request.append(QString(" WHERE %1=").arg(m_fieldBase.field(SQL_FIELD_KEY).name()));
	}

	int field = 0;
	int writedCount = 0;

	QSqlQuery query;

	if (query.exec("BEGIN TRANSACTION") == false)
	{
		return 0;
	}

	for (int r = 0; r < count; r++)
	{
		// for append record - request, or for update record - request + QString("%1").arg(key[r]
		//
		if (query.prepare(key == nullptr ? request :  request + QString("%1").arg(key[r])) == false)
		{
			continue;
		}

		field = 0;

		query.bindValue(field++, m_info.objectID());

		switch(m_info.objectType())
		{
			case SQL_TABLE_DATABASE_INFO:
				{
					SqlObjectInfo* info = static_cast<SqlObjectInfo*> (pRecord) + r;
					if (info == nullptr)
					{
						break;
					}

					query.bindValue(field++, info->objectID());
					query.bindValue(field++, info->caption());
					query.bindValue(field++, info->version());
				}
				break;

			case SQL_TABLE_PICTURE_INFO:
				{
					DataPacket* packet = static_cast<DataPacket*> (pRecord) + r;
					if (packet == nullptr)
					{
						break;
					}

					if (key == nullptr)
					{
						// for append record
						//
						packet->setPacketID(lastKey() + 1);
					}

					query.bindValue(field++, packet->packetID());

                    query.bindValue(field++, packet->hash());
					query.bindValue(field++, packet->packetTime());

					query.bindValue(field++, packet->imageWidth());
					query.bindValue(field++, packet->imageHeight());

                    query.bindValue(field++, packet->cmpPercent());
				}
				break;

			case SQL_TABLE_PICTURE:
				{
					DataPacket* packet = static_cast<DataPacket*> (pRecord) + r;
					if (packet == nullptr)
					{
						break;
					}

					query.bindValue(field++, packet->packetID());	// pictureID() was created by SQL_TABLE_PICTURE_INFO

					query.bindValue(field++, packet->imageData());
				}
				break;

			default:
				assert(0);
		}

		if (query.exec() == false)
		{
			qDebug() << __FUNCTION__ << query.lastError().text();

			continue;
		}

		writedCount ++;
	}

	if (query.exec("COMMIT") == false)
	{
		return 0;
	}

	return writedCount;
}

// -------------------------------------------------------------------------------------------------------------------

int SqlTable::remove(const int* key, int keyCount) const
{
	if (isOpen() == false)
	{
		return 0;
	}

	if (key == nullptr || keyCount == 0)
	{
		return 0;
	}

	QString keyFieldName = m_fieldBase.field(SQL_FIELD_KEY).name();
	if (keyFieldName.isEmpty() == true)
	{
		return 0;
	}

	int count = recordCount();
	if (count == 0)
	{
		return 0;
	}

	int transactionCount = keyCount / REMOVE_TRANSACTION_RECORD_COUNT;

	if (keyCount % REMOVE_TRANSACTION_RECORD_COUNT != 0)
	{
		transactionCount++;
	}

	int record = 0;

	for (int t = 0; t < transactionCount; t++)
	{
		QString request = QString("DELETE FROM %1 WHERE ").arg(m_info.caption());

		for (int k = 0; k < REMOVE_TRANSACTION_RECORD_COUNT; k++)
		{
			request.append(QString("%1=%2").arg(keyFieldName).arg(key[record++]));

			if (record >= keyCount )
			{
				break;
			}

			if (k != REMOVE_TRANSACTION_RECORD_COUNT - 1)
			{
				request.append(" OR ");
			}
		}

		QSqlQuery query;

		if (query.exec("BEGIN TRANSACTION") == false)
		{
			return 0;
		}

		if (query.exec(request) == false)
		{
			query.exec("END TRANSACTION");

			return 0;
		}

		if (query.exec("COMMIT") == false)
		{
			return 0;
		}
	}

	return count - recordCount();
}

// -------------------------------------------------------------------------------------------------------------------

SqlTable& SqlTable::operator=(SqlTable& from)
{
	m_pDatabase = from.m_pDatabase;
	m_info = from.m_info;
	m_fieldBase = from.m_fieldBase;

	return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

Database::Database(QObject* parent) :
	QObject(parent)
{
}

// -------------------------------------------------------------------------------------------------------------------

Database::~Database()
{
}

// -------------------------------------------------------------------------------------------------------------------

bool Database::open()
{
	bool result = false;

    //m_databaseOption.setType(OT::DatabaseType::SQLite);

	switch(m_databaseOption.type())
	{
		case OT::DatabaseType::SQLite:		result = openSQLite();		break;
		case OT::DatabaseType::PostgreSQL:	result = openPostgres();	break;

		default:
			assert(0);
			QMessageBox::critical(nullptr, tr("Database"), tr("Unknown type of database!"));
			return false;
	}

	if (result == false)
	{
		return false;
	}

	for(int type = 0; type < SQL_TABLE_COUNT; type++)
	{
		m_table[type].init(type, &m_database);
	}

	//
	//
	m_currentVersion = initVersion();
	if (m_currentVersion > DATABASE_VERSION)
	{
		QMessageBox::critical(nullptr, tr("Database"), tr("Loaded database has version %1, latest known version is %2, please update your software!")
							  .arg(m_currentVersion).arg(DATABASE_VERSION));

		close();

		return false;
	}

	createTables();

	//
	//
	result = applyMigrations();
	if (result == false)
	{
		QMessageBox::critical(nullptr, tr("Database"), tr("Error of migrations - Loaded database has version %1, latest known version is %2.")
							  .arg(m_currentVersion).arg(DATABASE_VERSION));

		close();

		return false;
	}

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

bool Database::openSQLite()
{
	//
	//
	m_database = QSqlDatabase::addDatabase("QSQLITE");
	if (m_database.lastError().isValid() == true)
	{
		return false;
	}

	//
	//
	QString path = QDir::currentPath();
	//QString path = m_databaseOption.locationPath();
	if (path.isEmpty() == true)
	{
		QMessageBox::critical(nullptr, tr("Database"), tr("Invalid path to Database!"));
		return false;
	}

	//
	//
	m_database.setDatabaseName(path + QDir::separator() + DATABASE_NAME + ".db");
	if (m_database.open() == false)
	{
		qDebug() << m_database.lastError().text();
		QMessageBox::critical(nullptr, tr("Database"), tr("Cannot open database"));
		return false;
	}

	//
	//
	QSqlQuery query;

	if (query.exec("PRAGMA foreign_keys=on") == false)
	{
		QMessageBox::critical(nullptr, tr("Database"), tr("Error set option of database: [foreign keys=on]"));
	}

	if (query.exec("PRAGMA synchronous=normal") == false)
	{
		QMessageBox::critical(nullptr, tr("Database"), tr("Error set option of database: [synchronous=normal]"));
	}

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

bool Database::openPostgres()
{
	QString connectionName = "default";
	{
		//
		//
		QSqlDatabase psql_db = QSqlDatabase::addDatabase("QPSQL", connectionName);
		if (psql_db.lastError().isValid() == true)
		{
			return false;
		}

		// set options postgres database
		//
		psql_db.setDatabaseName("postgres");
		psql_db.setHostName(m_databaseOption.ip());
		psql_db.setPort(m_databaseOption.port());
		psql_db.setUserName(m_databaseOption.user());
		psql_db.setPassword(m_databaseOption.password());

		if (psql_db.open() == false)
		{
			qDebug() << psql_db.lastError().text();
			QMessageBox::critical(nullptr, tr("Database"), tr("Cannot open database\n\n") + psql_db.lastError().text());
			return false;
		}

		//
		//
		QSqlQuery query(psql_db);

		bool result = query.exec("SELECT datname FROM pg_database");
		if (result == false)
		{
			qDebug() << query.lastError().text();
			psql_db.close();
			return false;
		}

		// find our database
		//
		bool isExist = false;

		while (query.next())
		{
			QString databaseName = query.value(0).toString();
			if (QString::compare(databaseName, DATABASE_NAME, Qt::CaseInsensitive) == 0)
			{
				isExist = true;
				break;
			}
		}

		// if our database was not found
		// create our database
		//
		if (isExist == false)
		{
			result = query.exec("CREATE DATABASE " + QString(DATABASE_NAME));
			if (result == false)
			{
				qDebug() << query.lastError().text();
				psql_db.close();
				return false;
			}
		}

		psql_db.close();
	}
	QSqlDatabase::removeDatabase(connectionName);

	//
	//
	m_database = QSqlDatabase::addDatabase("QPSQL");
	if (m_database.lastError().isValid() == true)
	{
		return false;
	}

	// set options our database
	//
	m_database.setDatabaseName(QString(DATABASE_NAME).toLower());
	m_database.setHostName(m_databaseOption.ip());
	m_database.setPort(m_databaseOption.port());
	m_database.setUserName(m_databaseOption.user());
	m_database.setPassword(m_databaseOption.password());

	if (m_database.open() == false)
	{
		qDebug() << m_database.lastError().text();
		QMessageBox::critical(nullptr, tr("Database"), tr("Cannot open database"));
		return false;
	}

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

void Database::close()
{
	for(int type = 0; type < SQL_TABLE_COUNT; type++)
	{
		if (m_table[type].isOpen() == true)
		{
			m_table[type].close();
		}

		m_table[type].info().clear();
	}

	if (m_database.isOpen() == true)
	{
		m_database.close();
	}
}

// -------------------------------------------------------------------------------------------------------------------

SqlTable* Database::openTable(int objectType)
{
	if (objectType < 0 || objectType >= SQL_TABLE_COUNT)
	{
		return nullptr;
	}

	if (m_table[objectType].isOpen() == true)
	{
		return nullptr;
	}

	if (m_table[objectType].open() == false)
	{
		return nullptr;
	}

	return &m_table[objectType];
}

// -------------------------------------------------------------------------------------------------------------------

int Database::initVersion()
{
	SqlTable table;
	if (table.init(SQL_TABLE_DATABASE_INFO, &m_database) == false)
	{
		return 0;
	}

	int databaseVersion = 0;

	std::vector<SqlObjectInfo> info;

	if (table.isExist() == false)
	{
		if (table.create() == true)
		{
			info.resize(SQL_TABLE_COUNT);

			for(int t = 0; t < SQL_TABLE_COUNT; t++)
			{
				info[static_cast<quint64>(t)] = m_table[t].info();
			}

			table.write(info.data(), static_cast<int>(info.size()));

			databaseVersion = DATABASE_VERSION;
		}
	}
	else
	{
		// open table that has data of database
		//
		if (table.open() == true)
		{
			// determine count of obejects in database
			//
			info.resize(static_cast<quint64>(table.recordCount()));

			int objectCount = table.read(info.data());
			for (int i = 0; i < objectCount; i++)
			{
				if (info[i].objectID() == SQL_TABLE_DATABASE_INFO)
				{
					databaseVersion = info[i].version();
					break;
				}
			}
		}
	}

	table.close();

	return databaseVersion;
}

// -------------------------------------------------------------------------------------------------------------------

void Database::createTables()
{
	// find table in database, if table is not exist, then create it
	//
	for(int type = 0; type < SQL_TABLE_COUNT; type++)
	{
		SqlTable table;

		if (table.init(type, &m_database) == true)
		{
			if (table.isExist() == false)
			{
				if (table.create() == false)
				{
					QMessageBox::critical(nullptr, tr("Database"), tr("Cannot create table: %1").arg(table.info().caption()));
				}
			}
		}
	}
}

// -------------------------------------------------------------------------------------------------------------------

bool Database::appendPicture(DataPacket* pPacket)
{
	if (pPacket == nullptr)
	{
		return false;
	}

	bool result = false;

	// save image details
	//
	SqlTable& table_pi = m_table[SQL_TABLE_PICTURE_INFO];
	if (table_pi.open() == false)
	{
		return false;
	}

	if (table_pi.write(pPacket) == 1)
	{
		result = true;
	}

	table_pi.close();

	if (result == false)
	{
		return false;
	}

	// save image
	//
	SqlTable& table_pd = m_table[SQL_TABLE_PICTURE];
	if (table_pd.open() == false)
	{
		return false;
	}

	if (table_pd.write(pPacket) == 1)
	{
		result = true;
	}

	table_pd.close();

	// clear image data
	//
	if (result == true)
	{
		pPacket->imageData().clear();
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

void Database::appendToBase(DataPacket* pPacket)
{
	if (pPacket == nullptr)
	{
		return;
	}

	bool result = appendPicture(pPacket);
	if (result == false)
	{
        //QMessageBox::critical(nullptr, tr("Save pictures"), tr("Error saving pictures to database"));
		return;
	}
}

// -------------------------------------------------------------------------------------------------------------------

bool Database::removePicture(const std::vector<int>& keyList)
{
	// remove image datails
	//
	SqlTable& table_pi = m_table[SQL_TABLE_PICTURE_INFO];
	if (table_pi.open() == false)
	{
		return false;
	}

	bool result = true;

	if (table_pi.remove(keyList.data(), static_cast<int>(keyList.size())) != static_cast<int>(keyList.size()))
	{
		result &= false;
	}

	table_pi.close();

	// remove image
	//
	SqlTable& table_pd = m_table[SQL_TABLE_PICTURE];
	if (table_pd.open() == false)
	{
		return false;
	}

	if (table_pd.remove(keyList.data(), static_cast<int>(keyList.size())) != static_cast<int>(keyList.size()))
	{
		result &= false;
	}

	table_pd.close();

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

bool Database::updatePicture(const std::vector<DataPacket*>& list)
{
	SqlTable& table = m_table[SQL_TABLE_PICTURE_INFO];
	if (table.open() == false)
	{
		return false;
	}

	bool result = false;

	std::vector<int> keyList;
	std::vector<DataPacket> packetList;

	for (DataPacket* pPacket : list)
	{
		if (pPacket == nullptr)
		{
			continue;
		}

		packetList.push_back(*dynamic_cast<DataPacket*>(pPacket));
		keyList.push_back(pPacket->packetID());
	}

	if (table.write((void*) packetList.data(), static_cast<int>(keyList.size()), keyList.data()) == static_cast<int>(keyList.size()))
	{
		result = true;
	}

	table.close();

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

void Database::removeFromBase(const std::vector<int>& keyList)
{
	bool result = removePicture(keyList);
	if (result == false)
	{
		QMessageBox::critical(nullptr, tr("Delete pictures"), tr("Error remove pictures from database"));
	}
}

// -------------------------------------------------------------------------------------------------------------------

void Database::updateInBase(const std::vector<DataPacket*>& list)
{
	bool result = updatePicture(list);
	if (result == false)
	{
		QMessageBox::critical(nullptr, tr("Update pictures"), tr("Error update pictures from database"));
	}
}

// -------------------------------------------------------------------------------------------------------------------

bool Database::applyMigrations()
{
	if (m_currentVersion == DATABASE_VERSION)
	{
		return true;
	}

	// run all migrations
	//
	bool allMigrationOk = true;

	for(int i = m_currentVersion; i < DATABASE_VERSION; i++)
	{
		QSqlQuery query;
		bool result = query.exec(migration[i]);
		if (result == false)
		{
			qDebug() << query.lastError().text();
			allMigrationOk = false;
		}
	}

	if (allMigrationOk == false)
	{
		return false;
	}

	// update DatabaseInfo
	//
	SqlTable* pTable = openTable(SQL_TABLE_DATABASE_INFO);
	if (pTable != nullptr)
	{
		pTable->clear();

		std::vector<SqlObjectInfo> info;

		info.resize(SQL_TABLE_COUNT);
		for(int t = 0; t < SQL_TABLE_COUNT; t++)
		{
			info[static_cast<quint64>(t)] = m_table[t].info();
		}

		pTable->write(info.data(), static_cast<int>(info.size()));

		pTable->close();
	}

	return true;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

