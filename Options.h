#ifndef OPTIONS_H
#define OPTIONS_H

#include <QObject>
#include <QMutex>
#include <QSettings>
#include <QDir>

// ==============================================================================================

#define WINDOW_GEOMETRY_OPTIONS_KEY "Options/Window/"

// ==============================================================================================

#define TIME_OPTIONS_KEY "Options/Time/"

// ----------------------------------------------------------------------------------------------

const int DEFAULT_TIMEOUT_SECONDS = 60;

// ----------------------------------------------------------------------------------------------

class TimeOption
{

public:

    TimeOption();
    TimeOption(int seconds);
    virtual ~TimeOption() {}

public:

    int seconds() const { return m_seconds; }
    void setSeconds(int seconds) { m_seconds = seconds; }

	//
	//
	void load();
	void save();

private:

    int m_seconds = DEFAULT_TIMEOUT_SECONDS;
};

// ==============================================================================================

#define DATABASE_OPTIONS_REG_KEY "Options/Database/"

// ----------------------------------------------------------------------------------------------

namespace OT
{
	Q_NAMESPACE

	enum DatabaseType
	{
		SQLite = 0,
		PostgreSQL = 1,
	};
	Q_ENUM_NS(DatabaseType)

	const int DatabaseTypeCount = 2;

	#define ERR_DATABASE_TYPE(type) (static_cast<int>(type) < 0 || static_cast<int>(type) >= OT::DatabaseTypeCount)

	QString DatabaseTypeCaption(OT::DatabaseType type);
}

// ----------------------------------------------------------------------------------------------

const QString DEFAULT_DB_IP = "localhost";
const int DEFAULT_DB_PORT = 5432;
const QString DEFAULT_DB_USER = "postgres";
const QString DEFAULT_DB_PASSWORD = "password";

// ----------------------------------------------------------------------------------------------

class DatabaseOption : public QObject
{
	Q_OBJECT

public:

	explicit DatabaseOption(QObject* parent = nullptr);
	explicit DatabaseOption(const DatabaseOption& from, QObject* parent = nullptr);
	virtual ~DatabaseOption() override;

public:

	QString locationPath() const { return m_locationPath; }
	void setLocationPath(const QString& path) { m_locationPath = path; }

	OT::DatabaseType type() const { return m_type; }
	void setType(OT::DatabaseType type) { m_type = type; }


	QString ip() const { return m_ip; }
	void setIp(const QString& ip) { m_ip = ip; }

	int port() const { return m_port; }
	void setPort(int port) { m_port = port; }

	QString user() const { return m_user; }
	void setUser(const QString& user) { m_user = user; }

	QString password() const { return m_password; }
	void setPassword(const QString& password) { m_password = password; }

	//
	//
	void load();
	void save();

	//
	//
	DatabaseOption& operator=(const DatabaseOption& from);

private:

	QString m_locationPath;
    OT::DatabaseType m_type = OT::SQLite;

	QString m_ip = DEFAULT_DB_IP;
	int m_port = DEFAULT_DB_PORT;
	QString m_user = DEFAULT_DB_IP;
	QString m_password = DEFAULT_DB_IP;
};

// ==============================================================================================

class Options : public QObject
{
	Q_OBJECT

public:

	explicit Options(QObject* parent = nullptr);
	explicit Options(const Options& from, QObject* parent = nullptr);
	virtual ~Options() override;

public:

	DatabaseOption& database() { return m_database; }
	void setDatabase(const DatabaseOption& database) { m_database = database; }

    TimeOption& time() { return m_time; }
    void setTime(const TimeOption& time) { m_time = time; }

	//
	//
	void load();
	void save();

	//
	//
	Options& operator=(const Options& from);

private:

	QMutex m_mutex;

	DatabaseOption m_database;
    TimeOption m_time;
};

// ==============================================================================================

extern Options theOptions;

// ==============================================================================================

#endif // OPTIONS_H
