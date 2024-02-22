#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QtSql>

#include "PacketBase.h"
#include "Options.h"

// ==============================================================================================

#define					DATABASE_NAME		"ScreenSaver"
#define					DATABASE_VERSION	2

// ==============================================================================================

const char* const migration[DATABASE_VERSION] =
{
    "ALTER TABLE PicturesInfo ADD Brightness INTEGER;",			// 1
    "ALTER TABLE PicturesInfo ADD Location VARCHAR(256);",		// 2
};

// ==============================================================================================
//
// Is a list of fields SQL tables.
//
class SqlFieldBase : public QSqlRecord
{
public:

	SqlFieldBase();
	virtual ~SqlFieldBase() {}

public:

	int					init(int objectType, int version);

	void				append(const QSqlField& field);
	void				append(QString name, QMetaType::Type type = QMetaType::UnknownType, int length = 0);

	QString				extFieldName(int index);
};

// ==============================================================================================

const char* const		SqlTableName[] =
{
						"DatabaseInfo",
						"PicturesInfo",
						"Pictures",

};

const int				SQL_TABLE_COUNT							= sizeof(SqlTableName)/sizeof(SqlTableName[0]);

const int				SQL_TABLE_UNKNONW						= -1,
						SQL_TABLE_DATABASE_INFO					= 0,
                        SQL_TABLE_PICTURE_INFO					= 1,
                        SQL_TABLE_PICTURE						= 2;

#define ERR_SQL_TABLE_TYPE(type) (static_cast<int>(type) < 0 || static_cast<int>(type) >= SQL_TABLE_COUNT)

// ==============================================================================================

const int				SQL_TABLE_VER_UNKNONW = -1;

// ----------------------------------------------------------------------------------------------
//
// current versions of tables or views
//
const int				SqlTableVersion[SQL_TABLE_COUNT] =
{
						DATABASE_VERSION,	//	SQL_TABLE_DATABASE_INFO

                        0,					//	SQL_TABLE_PICTURE_INFO
						0,					//	SQL_TABLE_PICTURE
};

// ==============================================================================================

const int				SQL_OBJECT_ID_UNKNONW = -1;

// ----------------------------------------------------------------------------------------------
//
// unique object ID in the database
//
const int				SqlObjectID[SQL_TABLE_COUNT] =
{
						0,			//	SQL_TABLE_DATABASE_INFO

                        100,		//	SQL_TABLE_PICTURE_INFO
                        101,		//	SQL_TABLE_PICTURE
};

// ==============================================================================================

const int				SQL_FIELD_OBJECT_ID = 0;			// zero column is unique identifier of the table (or other object) in the database
const int				SQL_FIELD_KEY		= 1;			// first column is key in the table, for example:: RecordID, PointID, SignalID, ReportID и т.д.

// ==============================================================================================

const int				SQL_INVALID_INDEX	= -1;
const int				SQL_INVALID_KEY		= -1;
const int				SQL_INVALID_RECORD	= -1;

// ==============================================================================================
//
// Represents the structure determines the version of the object (tables, databases, etc.) in the database
//
class SqlObjectInfo
{
public:

	SqlObjectInfo();
	virtual ~SqlObjectInfo() {}

public:

	bool				init(int objectType);
	void				clear();

	int					objectType() const { return m_objectType; }
	void				setObjectType(int type) { m_objectType = type; }

	int					objectID() const { return m_objectID; }
	void				setObjectID(int objectID) { m_objectID = objectID; }

	QString				caption() const { return m_caption; }
	void				setCaption(const QString& caption) { m_caption = caption; }

	int					version() const { return m_version; }
	void				setVersion(int verison) { m_version = verison; }

	SqlObjectInfo&		operator=(SqlObjectInfo& from);

private:

	int					m_objectType = SQL_TABLE_UNKNONW;			// type of table
	int					m_objectID = SQL_OBJECT_ID_UNKNONW;			// unique identifier of table in the database
	QString				m_caption;									// caption of table
	int					m_version = SQL_TABLE_VER_UNKNONW;			// table version, is read when the database initialization
};

// ==============================================================================================

const int				REMOVE_TRANSACTION_RECORD_COUNT = 500;

// ----------------------------------------------------------------------------------------------

class SqlTable
{
public:

	SqlTable();
	virtual ~SqlTable();

public:

	SqlObjectInfo&		info() { return m_info; }
	void				setInfo(SqlObjectInfo info) { m_info = info; }

	bool				isEmpty() { return recordCount() == 0; }
	int					recordCount() const;
	int					lastKey() const;

	bool				init(int objectType, QSqlDatabase* pDatabase);

	bool				isExist() const;
	bool				isOpen() const { return m_fieldBase.count() != 0; }
	bool				open();
	void				close();

	bool				create();
	bool				drop();
	bool				clear();

	int					read(void* pRecord, int key) { return read(pRecord, &key, 1); }
	int					read(void* pRecord, int* key = nullptr, int keyCount = 0);			// read record form table, if key == nullptr in the array pRecord will be record all records of table

	int					write(void* pRecord) { return write(pRecord, 1); }
	int					write(void* pRecord, int count, int key) { return write(pRecord, count, &key); }
	int					write(void* pRecord, int recordCount, int* key = nullptr);			// insert or update records (depend from key) in a table, pRecord - array of record, count - amount records

	int					remove(int key) { return remove(&key, 1); }							// remove records by key
	int					remove(const int* key, int keyCount) const;

	SqlTable&			operator=(SqlTable& from);

private:

	QSqlDatabase*		m_pDatabase = nullptr;
	SqlObjectInfo		m_info;
	SqlFieldBase		m_fieldBase;
};

// ==============================================================================================

class Database : public QObject
{
	Q_OBJECT

public:

	explicit Database(QObject* parent = nullptr);
	virtual ~Database() override;

public:

	void				setDatabaseOption(const DatabaseOption& option) { m_databaseOption = option; }

	bool				isOpen() const { return m_database.isOpen(); }
	bool				open();
	bool				openSQLite();
	bool				openPostgres();
	void				close();

	SqlTable*			openTable(int objectType);

	bool				appendPicture(DataPacket* pPacket);
	bool				removePicture(const std::vector<int>& keyList);
	bool				updatePicture(const std::vector<DataPacket*>& list);

private:

	QSqlDatabase		m_database;
	SqlTable			m_table[SQL_TABLE_COUNT];

	DatabaseOption		m_databaseOption;

	int					m_currentVersion = 0;

	int					initVersion();
	void				createTables();

	bool				applyMigrations();

public slots:

	void				appendToBase(DataPacket* pPacket);
	void				removeFromBase(const std::vector<int>& keyList);
	void				updateInBase(const std::vector<DataPacket*>& list);
};

// ==============================================================================================

extern Database theDatabase;

// ==============================================================================================

#endif // DATABASE_H
