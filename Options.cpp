#include "Options.h"

// -------------------------------------------------------------------------------------------------------------------

Options theOptions;

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

TimeOption::TimeOption()
{
}

// -------------------------------------------------------------------------------------------------------------------

TimeOption::TimeOption(int seconds)
    : m_seconds(seconds)
{
}

// -------------------------------------------------------------------------------------------------------------------

void TimeOption::load()
{
	QSettings s;
    
    m_seconds = s.value(QString("%1Seconds").arg(TIME_OPTIONS_KEY), DEFAULT_TIMEOUT_SECONDS).toInt();
}

// -------------------------------------------------------------------------------------------------------------------

void TimeOption::save()
{
	QSettings s;

    s.setValue(QString("%1Seconds").arg(TIME_OPTIONS_KEY), m_seconds);
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

DatabaseOption::DatabaseOption(QObject* parent) :
	QObject(parent)
{
}

// -------------------------------------------------------------------------------------------------------------------

DatabaseOption::DatabaseOption(const DatabaseOption& from, QObject* parent) :
	QObject(parent)
{
	*this = from;
}

// -------------------------------------------------------------------------------------------------------------------

DatabaseOption::~DatabaseOption()
{
}

// -------------------------------------------------------------------------------------------------------------------

void DatabaseOption::load()
{
	QSettings s;

	m_locationPath = s.value(QString("%1LocationPath").arg(DATABASE_OPTIONS_REG_KEY), QDir::currentPath()).toString();
    m_type = static_cast<OT::DatabaseType>(s.value(QString("%1Type").arg(DATABASE_OPTIONS_REG_KEY), OT::SQLite).toInt());

	m_ip = s.value(QString("%1IP").arg(DATABASE_OPTIONS_REG_KEY), DEFAULT_DB_IP).toString();
	m_port = s.value(QString("%1Port").arg(DATABASE_OPTIONS_REG_KEY), DEFAULT_DB_PORT).toInt();
	m_user = s.value(QString("%1User").arg(DATABASE_OPTIONS_REG_KEY), DEFAULT_DB_USER).toString();
	m_password = s.value(QString("%1Password").arg(DATABASE_OPTIONS_REG_KEY), DEFAULT_DB_PASSWORD).toString();
}

// -------------------------------------------------------------------------------------------------------------------

void DatabaseOption::save()
{
	QSettings s;

	s.setValue(QString("%1LocationPath").arg(DATABASE_OPTIONS_REG_KEY), m_locationPath);
	s.setValue(QString("%1Type").arg(DATABASE_OPTIONS_REG_KEY), m_type);

	s.setValue(QString("%1IP").arg(DATABASE_OPTIONS_REG_KEY), m_ip);
	s.setValue(QString("%1Port").arg(DATABASE_OPTIONS_REG_KEY), m_port);
	s.setValue(QString("%1User").arg(DATABASE_OPTIONS_REG_KEY), m_user);
	s.setValue(QString("%1Password").arg(DATABASE_OPTIONS_REG_KEY), m_password);
}

// -------------------------------------------------------------------------------------------------------------------

DatabaseOption& DatabaseOption::operator=(const DatabaseOption& from)
{
	m_locationPath = from.m_locationPath;
	m_type = from.m_type;

	m_ip = from.m_ip;
	m_port = from.m_port;
	m_user = from.m_user;
	m_password = from.m_password;

	return *this;
}

// -------------------------------------------------------------------------------------------------------------------

QString OT::DatabaseTypeCaption(OT::DatabaseType type)
{
	QString caption;

	switch (type)
	{
		case OT::SQLite:		caption = "SQLite";		break;
		case OT::PostgreSQL:	caption = "PostgreSQL";	break;

		default:
			assert(0);
			caption = QObject::tr("Unknown");
	}

	return caption;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

Options::Options(QObject* parent) :
	QObject(parent)
{
}

// -------------------------------------------------------------------------------------------------------------------

Options::Options(const Options& from, QObject* parent) :
	QObject(parent)
{
	*this = from;
}

// -------------------------------------------------------------------------------------------------------------------

Options::~Options()
{
}

// -------------------------------------------------------------------------------------------------------------------

void Options::load()
{
	m_database.load();
    m_time.load();
}

// -------------------------------------------------------------------------------------------------------------------

void Options::save()
{
	m_database.save();
    m_time.save();
}

// -------------------------------------------------------------------------------------------------------------------

Options& Options::operator=(const Options& from)
{
	QMutexLocker l(&m_mutex);

	m_database = from.m_database;
    m_time = from.m_time;

	return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

