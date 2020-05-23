#pragma once

#include "window-dock.hpp"
#include "obs-app.hpp"
#include <QScopedPointer>

#include <browser-panel.hpp>
extern QCef *cef;
extern QCefCookieManager *panel_cookies;

class BrowserDock : public OBSDock {
public:
	inline BrowserDock() : OBSDock() { setAttribute(Qt::WA_NativeWindow); }

	QScopedPointer<QCefWidget> cefWidget;

	inline void SetWidget(QCefWidget *widget_)
	{
		setWidget(widget_);
		cefWidget.reset(widget_);

		QAction *reloadBtn = new QAction(this);
		reloadBtn->setProperty("themeID", "refreshIconSmall");
		// TODO Translate
		reloadBtn->setToolTip(QTStr("ExtraBrowsers.Reload"));
		connect(reloadBtn, &QAction::triggered, this,
			&BrowserDock::reloadPage, Qt::DirectConnection);
		static_cast<OBSDockTitle *>(this->titleBarWidget())
			->setButtons(QList<QAction *>({reloadBtn}));
	}

	void reloadPage();
	void closeEvent(QCloseEvent *event) override;
};
