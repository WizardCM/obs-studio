#include "window-dock.hpp"
#include "obs-app.hpp"

#include <QMessageBox>
#include <QCheckBox>

OBSDockTitle::OBSDockTitle(OBSDock *parent) : QFrame(parent), dock(parent)
{
	// TODO Try and reuse style attributes from normal dock heading
	int spacing = 6;
	titleLabel = new QLabel(parent->windowTitle(), this);
	titleLabel->setObjectName("title");
	titleLabel->setSizePolicy(QSizePolicy::Expanding,
				  QSizePolicy::Expanding);

	buttonsLayout = new QHBoxLayout();
	buttonsLayout->setSpacing(spacing);
	buttonsLayout->setMargin(0);

	QHBoxLayout *l = new QHBoxLayout(this);
	l->setSpacing(spacing);
	l->setMargin(1);
	l->addWidget(titleLabel, 1);
	l->addLayout(buttonsLayout);

	floatBtn = new QToolButton();
	floatBtn->setToolTip("Popout");
	floatBtn->setAutoRaise(true);
	l->addWidget(floatBtn);
	connect(floatBtn, &QToolButton::clicked, this,
		&OBSDockTitle::setFloating);

	closeBtn = new QToolButton();
	connect(closeBtn, &QToolButton::clicked, this,
		&OBSDockTitle::closeParent);
	closeBtn->setAutoRaise(true);
	closeBtn->setToolTip("Close");
	l->addWidget(closeBtn);

	setContextMenuPolicy(Qt::NoContextMenu);

	connect(parent, &QDockWidget::featuresChanged, this,
		&OBSDockTitle::featuresChanged, Qt::QueuedConnection);
	connect(parent, &QWidget::windowTitleChanged, titleLabel,
		&QLabel::setText);
}

void OBSDockTitle::SetCloseIcon(const QIcon &icon)
{
	closeIcon = icon;
	closeBtn->setIcon(closeIcon);
}

void OBSDockTitle::SetPopoutIcon(const QIcon &icon)
{
	popoutIcon = icon;
	floatBtn->setIcon(popoutIcon);
}

QIcon OBSDockTitle::GetCloseIcon() const
{
	return closeIcon;
}

QIcon OBSDockTitle::GetPopoutIcon() const
{
	return popoutIcon;
}

void OBSDockTitle::closeParent()
{
	dock->close();
}

void OBSDockTitle::setFloating()
{
	dock->setFloating(!dock->isFloating());
}

void OBSDockTitle::featuresChanged(QDockWidget::DockWidgetFeatures features)
{
	floatBtn->setVisible(
		features.testFlag(QDockWidget::DockWidgetFloatable));
	closeBtn->setVisible(
		features.testFlag(QDockWidget::DockWidgetClosable));
}

QList<QToolButton *> OBSDockTitle::getButtons() const
{
	return buttons;
}

void OBSDockTitle::setButtonPropertiesFromAction(QToolButton *button,
						 QAction *action)
{
	for (const QByteArray &name : action->dynamicPropertyNames()) {
		button->setProperty(name.constData(),
				    action->property(name.constData()));
	}
}

void OBSDockTitle::setButtons(QList<QAction *> actions)
{
	buttonActions = actions;

	for (auto &action : actions) {
		QToolButton *button = new QToolButton(this);
		button->setObjectName(action->objectName());
		setButtonPropertiesFromAction(button, action);
		connect(button, &QToolButton::clicked, action,
			&QAction::triggered);
		button->setToolTip(action->toolTip());
		button->setAutoRaise(true);
		buttonsLayout->addWidget(button);
		buttons.append(button);
	}
}

void OBSDock::dockedChanged(bool topLevel)
{
	// if (topLevel)
		// setTitleBarWidget(0);
	// else
	setTitleBarWidget(dockTitle);
}

void OBSDock::closeEvent(QCloseEvent *event)
{
	auto msgBox = []() {
		QMessageBox msgbox(App()->GetMainWindow());
		msgbox.setWindowTitle(QTStr("DockCloseWarning.Title"));
		msgbox.setText(QTStr("DockCloseWarning.Text"));
		msgbox.setIcon(QMessageBox::Icon::Information);
		msgbox.addButton(QMessageBox::Ok);

		QCheckBox *cb = new QCheckBox(QTStr("DoNotShowAgain"));
		msgbox.setCheckBox(cb);

		msgbox.exec();

		if (cb->isChecked()) {
			config_set_bool(App()->GlobalConfig(), "General",
					"WarnedAboutClosingDocks", true);
			config_save_safe(App()->GlobalConfig(), "tmp", nullptr);
		}
	};

	bool warned = config_get_bool(App()->GlobalConfig(), "General",
				      "WarnedAboutClosingDocks");
	if (!warned) {
		QMetaObject::invokeMethod(App(), "Exec", Qt::QueuedConnection,
					  Q_ARG(VoidFunc, msgBox));
	}

	QDockWidget::closeEvent(event);
}
