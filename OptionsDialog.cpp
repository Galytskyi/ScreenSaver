#include "OptionsDialog.h"

// -------------------------------------------------------------------------------------------------------------------

OptionsDialog::OptionsDialog(const TimeOption& timeOption, const DatabaseOption& databaseOption, QWidget* parent)
	: QDialog(parent)
    , m_timeOption(timeOption)
	, m_databaseOption(databaseOption)
{
	createInterface();
}

// -------------------------------------------------------------------------------------------------------------------

OptionsDialog::~OptionsDialog()
{
}

// -------------------------------------------------------------------------------------------------------------------

bool OptionsDialog::createInterface()
{
	setWindowFlags(Qt::Dialog | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint);
	setWindowIcon(QIcon(":/icons/Options.png"));
	setWindowTitle(tr("Options"));

	QRect screen = parentWidget()->screen()->availableGeometry();
    resize(static_cast<int>(screen.width() * 0.15), static_cast<int>(screen.height() * 0.1));
	move(screen.center() - rect().center());

    // time
	//
    QGroupBox* timeGroup = new QGroupBox(tr("Time for screenshots updating"));
    QVBoxLayout *timeLabelLayout = new QVBoxLayout;


    QLabel* timeLabel = new QLabel(tr("Update time (sec)"), this);
    timeLabelLayout->addWidget(timeLabel);

    QVBoxLayout *timeEditLayout = new QVBoxLayout;

    m_timeEdit = new QLineEdit(QString::number(m_timeOption.seconds()), this);

    timeEditLayout->addWidget(m_timeEdit);

    QHBoxLayout *timeLayout = new QHBoxLayout;

    timeLayout->addLayout(timeLabelLayout);
    timeLayout->addLayout(timeEditLayout);

    timeGroup->setLayout(timeLayout);

	// buttons
	//
	m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
	connect(m_buttonBox, &QDialogButtonBox::accepted, this, &OptionsDialog::onOk);
	connect(m_buttonBox, &QDialogButtonBox::rejected, this, &OptionsDialog::reject);

	// Main Layout
	//
	QVBoxLayout *mainLayout = new QVBoxLayout;

    mainLayout->addWidget(timeGroup);
	mainLayout->addStretch();
	mainLayout->addWidget(m_buttonBox);

	setLayout(mainLayout);

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

void OptionsDialog::onOk()
{
    // time
	//

    QString seconds = m_timeEdit->text();
    if (seconds.isEmpty() == true)
	{
        QMessageBox::information(nullptr, windowTitle(), tr("Field time is empty!"));
		return;
	}

    m_timeOption.setSeconds(seconds.toInt());

	accept();
}

// -------------------------------------------------------------------------------------------------------------------
