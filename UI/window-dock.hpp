#pragma once

#include <QDockWidget>
#include <QFrame>
#include <QToolButton>
#include <QHBoxLayout>
#include <QAction>
#include <QLabel>

class OBSDock;

class OBSDockTitle : public QFrame {
	Q_OBJECT
	Q_PROPERTY(QIcon closeIcon MEMBER closeIcon WRITE SetCloseIcon READ
			   GetCloseIcon DESIGNABLE true)
	Q_PROPERTY(QIcon popoutIcon MEMBER popoutIcon WRITE SetPopoutIcon READ
			   GetPopoutIcon DESIGNABLE true)

public:
	OBSDockTitle(OBSDock *parent = nullptr);

	QList<QToolButton *> getButtons() const;
	void setButtons(QList<QAction *> actions);

private:
	OBSDock *dock;
	QLabel *titleLabel;
	QList<QToolButton *> buttons;
	QList<QAction *> buttonActions;
	QToolButton *floatBtn;
	QToolButton *closeBtn;
	QHBoxLayout *buttonsLayout;

	QIcon closeIcon;
	QIcon popoutIcon;

	void SetCloseIcon(const QIcon &icon);
	QIcon GetCloseIcon() const;
	void SetPopoutIcon(const QIcon &icon);
	QIcon GetPopoutIcon() const;

	void setButtonPropertiesFromAction(QToolButton *button,
					   QAction *action);
public slots:
	void featuresChanged(QDockWidget::DockWidgetFeatures features);
	void setFloating();
	void closeParent();
};

class OBSDock : public QDockWidget {
	Q_OBJECT

public:
	inline OBSDock(QWidget *parent = nullptr) : QDockWidget(parent)
	{
		setTitleBarWidget(dockTitle = new OBSDockTitle(this));
		connect(this, &OBSDock::topLevelChanged, this,
			&OBSDock::dockedChanged);
	}

	OBSDockTitle *titleWidget() const;
	virtual void closeEvent(QCloseEvent *event);
private slots:
	void dockedChanged(bool topLevel);

private:
	OBSDockTitle *dockTitle;

	friend class OBSDockTitle;
};
