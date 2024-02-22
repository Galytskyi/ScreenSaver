#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

#include "Options.h"

#include <QDialog>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QRect>
#include <QScreen>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QMessageBox>

// ==============================================================================================

class OptionsDialog : public QDialog
{
	Q_OBJECT

public:

    explicit OptionsDialog(const TimeOption& timeOption, const DatabaseOption& databaseOption, QWidget *parent = nullptr);
	virtual ~OptionsDialog() override;

public:

    TimeOption& timeOption() { return m_timeOption; }
	DatabaseOption& databaseOption() { return m_databaseOption; }

private:

    TimeOption m_timeOption;
	DatabaseOption m_databaseOption;

    QLineEdit* m_timeEdit = nullptr;

	QDialogButtonBox* m_buttonBox = nullptr;

	bool createInterface();

private slots:

	void onOk();
};

// ==============================================================================================

#endif // OPTIONSDIALOG_H
