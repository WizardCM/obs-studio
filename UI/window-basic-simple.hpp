
#pragma once

#include <QBuffer>
#include <QAction>
#include <QWidgetAction>
#include <QSystemTrayIcon>
#include <QStyledItemDelegate>
#include <obs.hpp>
#include <vector>
#include <memory>
#include "window-main.hpp"
#include "window-basic-interaction.hpp"
#include "window-basic-properties.hpp"
#include "window-basic-transform.hpp"
#include "window-basic-adv-audio.hpp"
#include "window-basic-filters.hpp"
#include "window-missing-files.hpp"
#include "window-projector.hpp"
#include "window-basic-about.hpp"
#include "auth-base.hpp"
#include "log-viewer.hpp"
#include "undo-stack-obs.hpp"

#include <obs-frontend-internal.hpp>

#include <util/platform.h>
#include <util/threading.h>
#include <util/util.hpp>

#include <QPointer>

#ifndef WINDOWBASISIMPLE_H
#define WINDOWBASISIMPLE_H

#include <QMainWindow>

#define PREVIEW_EDGE_SIZE 10


namespace Ui {
class WindowBasicSimple;
}

class WindowBasicSimple : public OBSMainWindow
{
    Q_OBJECT
	friend class OBSBasicPreview;

public:
    explicit WindowBasicSimple(QWidget *parent = nullptr);
    ~WindowBasicSimple();

	virtual void OBSInit() override;

	virtual config_t *Config() const override;

	virtual int GetProfilePath(char *path, size_t size,
				   const char *file) const override;

	inline bool IsPreviewProgramMode() const
	{
		return os_atomic_load_bool(&previewProgramMode);
	}

	inline void GetDisplayRect(int &x, int &y, int &cx, int &cy)
	{
		x = previewX;
		y = previewY;
		cx = previewCX;
		cy = previewCY;
	}

private:
    Ui::WindowBasicSimple *ui;
	QPointer<QObject> shortcutFilter;
	static void RenderMain(void *data, uint32_t cx, uint32_t cy);

	ConfigFile basicConfig;

	QByteArray startingDockLayout;
	bool previewEnabled = true;

	int previewX = 0, previewY = 0;
	int previewCX = 0, previewCY = 0;
	float previewScale = 0.0f;
	volatile bool previewProgramMode = false;

private slots:

	void EnablePreview();
	void DisablePreview();

	void ResizePreview(uint32_t cx, uint32_t cy);

	void EnablePreviewDisplay(bool enable);
	void TogglePreview();
};

#endif // WINDOWBASISIMPLE_H
