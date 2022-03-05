#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <cmath>
#include "qt-wrappers.hpp"
#include "obs-app.hpp"
#include "adv-audio-control.hpp"
#include "window-basic-main.hpp"

#ifndef NSEC_PER_MSEC
#define NSEC_PER_MSEC 1000000
#endif

#define MIN_DB -96.0
#define MAX_DB 26.0

OBSAdvAudioCtrl::OBSAdvAudioCtrl(QWidget *parent, obs_source_t *source_)
	: QWidget(parent), source(source_), ui(new Ui::OBSAdvAudioCtrl)
{
	signal_handler_t *handler = obs_source_get_signal_handler(source);
	QString sourceName = QT_UTF8(obs_source_get_name(source));
	float vol = obs_source_get_volume(source);
	uint32_t flags = obs_source_get_flags(source);
	uint32_t mixers = obs_source_get_audio_mixers(source);

	ui->setupUi(this);

	ui->monitoringType->setVisible(obs_audio_monitoring_available());

	activateSignal.Connect(handler, "activate", OBSSourceActivated, this);
	deactivateSignal.Connect(handler, "deactivate", OBSSourceDeactivated,
				 this);
	volChangedSignal.Connect(handler, "volume", OBSSourceVolumeChanged,
				 this);
	syncOffsetSignal.Connect(handler, "audio_sync", OBSSourceSyncChanged,
				 this);
	flagsSignal.Connect(handler, "update_flags", OBSSourceFlagsChanged,
			    this);
	if (obs_audio_monitoring_available())
		monitoringTypeSignal.Connect(handler, "audio_monitoring",
					     OBSSourceMonitoringTypeChanged,
					     this);
	mixersSignal.Connect(handler, "audio_mixers", OBSSourceMixersChanged,
			     this);
	balChangedSignal.Connect(handler, "audio_balance",
				 OBSSourceBalanceChanged, this);

	OBSBasic *main = reinterpret_cast<OBSBasic *>(App()->GetMainWindow());

	QIcon sourceIcon = main->GetSourceIcon(obs_source_get_id(source));
	QPixmap pixmap = sourceIcon.pixmap(QSize(16, 16));
	ui->iconLabel->setPixmap(pixmap);

	ui->nameLabel->setText(sourceName);

	bool isActive = obs_source_active(source);
	ui->active->setText(isActive ? QTStr("Basic.Stats.Status.Active")
				     : QTStr("Basic.Stats.Status.Inactive"));
	if (isActive)
		setThemeID(ui->active, "error");

	ui->volume->setMinimum(MIN_DB - 0.1);
	ui->volume->setMaximum(MAX_DB);
	ui->volume->setValue(obs_mul_to_db(vol));
	ui->volume->setAccessibleName(
		QTStr("Basic.AdvAudio.VolumeSource").arg(sourceName));

	if (ui->volume->value() < MIN_DB) {
		ui->volume->setSpecialValueText("-inf dB");
		ui->volume->setAccessibleDescription("-inf dB");
	}

	ui->percent->setValue((int)(obs_source_get_volume(source) * 100.0f));
	ui->percent->setAccessibleName(
		QTStr("Basic.AdvAudio.VolumeSource").arg(sourceName));

	VolumeType volType = (VolumeType)config_get_int(
		GetGlobalConfig(), "BasicWindow", "AdvAudioVolumeType");

	SetVolumeWidget(volType);

	ui->forceMono->setChecked((flags & OBS_SOURCE_FLAG_FORCE_MONO) != 0);
	ui->forceMono->setAccessibleName(
		QTStr("Basic.AdvAudio.MonoSource").arg(sourceName));

	// TODO Should be able to do in XML too
	ui->balance->setOrientation(Qt::Horizontal);
	ui->balance->setMinimum(0);
	ui->balance->setMaximum(100);
	ui->balance->setTickPosition(QSlider::TicksAbove);
	ui->balance->setTickInterval(50);
	ui->balance->setAccessibleName(
		QTStr("Basic.AdvAudio.BalanceSource").arg(sourceName));

	const char *speakers =
		config_get_string(main->Config(), "Audio", "ChannelSetup");

	ui->balance->setEnabled(strcmp(speakers, "Mono") != 0);

	float bal = obs_source_get_balance_value(source) * 100.0f;
	ui->balance->setValue((int)bal);

	int64_t cur_sync = obs_source_get_sync_offset(source);
	ui->syncOffset->setValue(int(cur_sync / NSEC_PER_MSEC));
	ui->syncOffset->setAccessibleName(
		QTStr("Basic.AdvAudio.SyncOffsetSource").arg(sourceName));

	int idx;
	if (obs_audio_monitoring_available()) {
		ui->monitoringType->addItem(
			QTStr("Basic.AdvAudio.Monitoring.None"),
			(int)OBS_MONITORING_TYPE_NONE);
		ui->monitoringType->addItem(
			QTStr("Basic.AdvAudio.Monitoring.MonitorOnly"),
			(int)OBS_MONITORING_TYPE_MONITOR_ONLY);
		ui->monitoringType->addItem(
			QTStr("Basic.AdvAudio.Monitoring.Both"),
			(int)OBS_MONITORING_TYPE_MONITOR_AND_OUTPUT);
		int mt = (int)obs_source_get_monitoring_type(source);
		idx = ui->monitoringType->findData(mt);
		ui->monitoringType->setCurrentIndex(idx);
		ui->monitoringType->setAccessibleName(
			QTStr("Basic.AdvAudio.MonitoringSource")
				.arg(sourceName));
	}

	ui->mixer1->setChecked(mixers & (1 << 0));
	ui->mixer2->setChecked(mixers & (1 << 1));
	ui->mixer3->setChecked(mixers & (1 << 2));
	ui->mixer4->setChecked(mixers & (1 << 3));
	ui->mixer5->setChecked(mixers & (1 << 4));
	ui->mixer6->setChecked(mixers & (1 << 5));

	speaker_layout sl = obs_source_get_speaker_layout(source);

	if (sl == SPEAKERS_STEREO) {
		ui->labelL->setVisible(true);
		ui->balance->setVisible(true);
		ui->labelR->setVisible(true);
		ui->balanceContainer->setMaximumWidth(170);
	}

	QWidget::connect(ui->volume, SIGNAL(valueChanged(double)), this,
			 SLOT(volumeChanged(double)));
	QWidget::connect(ui->percent, SIGNAL(valueChanged(int)), this,
			 SLOT(percentChanged(int)));
	QWidget::connect(ui->forceMono, SIGNAL(clicked(bool)), this,
			 SLOT(downmixMonoChanged(bool)));
	QWidget::connect(ui->balance, SIGNAL(valueChanged(int)), this,
			 SLOT(balanceChanged(int)));
	QWidget::connect(ui->balance, SIGNAL(doubleClicked()), this,
			 SLOT(ResetBalance()));
	QWidget::connect(ui->syncOffset, SIGNAL(valueChanged(int)), this,
			 SLOT(syncOffsetChanged(int)));
	if (obs_audio_monitoring_available())
		QWidget::connect(ui->monitoringType,
				 SIGNAL(currentIndexChanged(int)), this,
				 SLOT(monitoringTypeChanged(int)));
	QWidget::connect(ui->mixer1, SIGNAL(clicked(bool)), this,
			 SLOT(mixer1Changed(bool)));
	QWidget::connect(ui->mixer2, SIGNAL(clicked(bool)), this,
			 SLOT(mixer2Changed(bool)));
	QWidget::connect(ui->mixer3, SIGNAL(clicked(bool)), this,
			 SLOT(mixer3Changed(bool)));
	QWidget::connect(ui->mixer4, SIGNAL(clicked(bool)), this,
			 SLOT(mixer4Changed(bool)));
	QWidget::connect(ui->mixer5, SIGNAL(clicked(bool)), this,
			 SLOT(mixer5Changed(bool)));
	QWidget::connect(ui->mixer6, SIGNAL(clicked(bool)), this,
			 SLOT(mixer6Changed(bool)));
}

/* ------------------------------------------------------------------------- */
/* OBS source callbacks */

void OBSAdvAudioCtrl::OBSSourceActivated(void *param, calldata_t *calldata)
{
	QMetaObject::invokeMethod(reinterpret_cast<OBSAdvAudioCtrl *>(param),
				  "SourceActiveChanged", Q_ARG(bool, true));
	UNUSED_PARAMETER(calldata);
}

void OBSAdvAudioCtrl::OBSSourceDeactivated(void *param, calldata_t *calldata)
{
	QMetaObject::invokeMethod(reinterpret_cast<OBSAdvAudioCtrl *>(param),
				  "SourceActiveChanged", Q_ARG(bool, false));
	UNUSED_PARAMETER(calldata);
}

void OBSAdvAudioCtrl::OBSSourceFlagsChanged(void *param, calldata_t *calldata)
{
	uint32_t flags = (uint32_t)calldata_int(calldata, "flags");
	QMetaObject::invokeMethod(reinterpret_cast<OBSAdvAudioCtrl *>(param),
				  "SourceFlagsChanged", Q_ARG(uint32_t, flags));
}

void OBSAdvAudioCtrl::OBSSourceVolumeChanged(void *param, calldata_t *calldata)
{
	float volume = (float)calldata_float(calldata, "volume");
	QMetaObject::invokeMethod(reinterpret_cast<OBSAdvAudioCtrl *>(param),
				  "SourceVolumeChanged", Q_ARG(float, volume));
}

void OBSAdvAudioCtrl::OBSSourceSyncChanged(void *param, calldata_t *calldata)
{
	int64_t offset = calldata_int(calldata, "offset");
	QMetaObject::invokeMethod(reinterpret_cast<OBSAdvAudioCtrl *>(param),
				  "SourceSyncChanged", Q_ARG(int64_t, offset));
}

void OBSAdvAudioCtrl::OBSSourceMonitoringTypeChanged(void *param,
						     calldata_t *calldata)
{
	int type = calldata_int(calldata, "type");
	QMetaObject::invokeMethod(reinterpret_cast<OBSAdvAudioCtrl *>(param),
				  "SourceMonitoringTypeChanged",
				  Q_ARG(int, type));
}

void OBSAdvAudioCtrl::OBSSourceMixersChanged(void *param, calldata_t *calldata)
{
	uint32_t mixers = (uint32_t)calldata_int(calldata, "mixers");
	QMetaObject::invokeMethod(reinterpret_cast<OBSAdvAudioCtrl *>(param),
				  "SourceMixersChanged",
				  Q_ARG(uint32_t, mixers));
}

void OBSAdvAudioCtrl::OBSSourceBalanceChanged(void *param, calldata_t *calldata)
{
	int balance = (float)calldata_float(calldata, "balance") * 100.0f;
	QMetaObject::invokeMethod(reinterpret_cast<OBSAdvAudioCtrl *>(param),
				  "SourceBalanceChanged", Q_ARG(int, balance));
}

/* ------------------------------------------------------------------------- */
/* Qt event queue source callbacks */

static inline void setCheckboxState(QCheckBox *checkbox, bool checked)
{
	checkbox->blockSignals(true);
	checkbox->setChecked(checked);
	checkbox->blockSignals(false);
}

void OBSAdvAudioCtrl::SourceActiveChanged(bool isActive)
{
	if (isActive) {
		ui->active->setText(QTStr("Basic.Stats.Status.Active"));
		setThemeID(ui->active, "error");
	} else {
		ui->active->setText(QTStr("Basic.Stats.Status.Inactive"));
		setThemeID(ui->active, "");
	}
}

void OBSAdvAudioCtrl::SourceFlagsChanged(uint32_t flags)
{
	bool forceMonoVal = (flags & OBS_SOURCE_FLAG_FORCE_MONO) != 0;
	setCheckboxState(ui->forceMono, forceMonoVal);
}

void OBSAdvAudioCtrl::SourceVolumeChanged(float value)
{
	ui->volume->blockSignals(true);
	ui->percent->blockSignals(true);
	ui->volume->setValue(obs_mul_to_db(value));
	ui->percent->setValue((int)std::round(value * 100.0f));
	ui->percent->blockSignals(false);
	ui->volume->blockSignals(false);
}

void OBSAdvAudioCtrl::SourceBalanceChanged(int value)
{
	ui->balance->blockSignals(true);
	ui->balance->setValue(value);
	ui->balance->blockSignals(false);
}

void OBSAdvAudioCtrl::SourceSyncChanged(int64_t offset)
{
	ui->syncOffset->blockSignals(true);
	ui->syncOffset->setValue(offset / NSEC_PER_MSEC);
	ui->syncOffset->blockSignals(false);
}

void OBSAdvAudioCtrl::SourceMonitoringTypeChanged(int type)
{
	int idx = ui->monitoringType->findData(type);
	ui->monitoringType->blockSignals(true);
	ui->monitoringType->setCurrentIndex(idx);
	ui->monitoringType->blockSignals(false);
}

void OBSAdvAudioCtrl::SourceMixersChanged(uint32_t mixers)
{
	setCheckboxState(ui->mixer1, mixers & (1 << 0));
	setCheckboxState(ui->mixer2, mixers & (1 << 1));
	setCheckboxState(ui->mixer3, mixers & (1 << 2));
	setCheckboxState(ui->mixer4, mixers & (1 << 3));
	setCheckboxState(ui->mixer5, mixers & (1 << 4));
	setCheckboxState(ui->mixer6, mixers & (1 << 5));
}

/* ------------------------------------------------------------------------- */
/* Qt control callbacks */

void OBSAdvAudioCtrl::volumeChanged(double db)
{
	float prev = obs_source_get_volume(source);

	if (db < MIN_DB) {
		ui->volume->setSpecialValueText("-inf dB");
		db = -INFINITY;
	}

	float val = obs_db_to_mul(db);
	obs_source_set_volume(source, val);

	auto undo_redo = [](const std::string &name, float val) {
		OBSSourceAutoRelease source =
			obs_get_source_by_name(name.c_str());
		obs_source_set_volume(source, val);
	};

	const char *name = obs_source_get_name(source);

	OBSBasic *main = OBSBasic::Get();
	main->undo_s.add_action(
		QTStr("Undo.Volume.Change").arg(name),
		std::bind(undo_redo, std::placeholders::_1, prev),
		std::bind(undo_redo, std::placeholders::_1, val), name, name,
		true);
}

void OBSAdvAudioCtrl::percentChanged(int percent)
{
	float prev = obs_source_get_volume(source);
	float val = (float)percent / 100.0f;

	obs_source_set_volume(source, val);

	auto undo_redo = [](const std::string &name, float val) {
		OBSSourceAutoRelease source =
			obs_get_source_by_name(name.c_str());
		obs_source_set_volume(source, val);
	};

	const char *name = obs_source_get_name(source);
	OBSBasic::Get()->undo_s.add_action(
		QTStr("Undo.Volume.Change").arg(name),
		std::bind(undo_redo, std::placeholders::_1, prev),
		std::bind(undo_redo, std::placeholders::_1, val), name, name,
		true);
}

static inline void set_mono(obs_source_t *source, bool mono)
{
	uint32_t flags = obs_source_get_flags(source);
	if (mono)
		flags |= OBS_SOURCE_FLAG_FORCE_MONO;
	else
		flags &= ~OBS_SOURCE_FLAG_FORCE_MONO;
	obs_source_set_flags(source, flags);
}

void OBSAdvAudioCtrl::downmixMonoChanged(bool val)
{
	uint32_t flags = obs_source_get_flags(source);
	bool forceMonoActive = (flags & OBS_SOURCE_FLAG_FORCE_MONO) != 0;

	if (forceMonoActive == val)
		return;

	if (val)
		flags |= OBS_SOURCE_FLAG_FORCE_MONO;
	else
		flags &= ~OBS_SOURCE_FLAG_FORCE_MONO;

	obs_source_set_flags(source, flags);

	auto undo_redo = [](const std::string &name, bool val) {
		OBSSourceAutoRelease source =
			obs_get_source_by_name(name.c_str());
		set_mono(source, val);
	};

	QString text = QTStr(val ? "Undo.ForceMono.On" : "Undo.ForceMono.Off");

	const char *name = obs_source_get_name(source);
	OBSBasic::Get()->undo_s.add_action(
		text.arg(name),
		std::bind(undo_redo, std::placeholders::_1, !val),
		std::bind(undo_redo, std::placeholders::_1, val), name, name);
}

void OBSAdvAudioCtrl::balanceChanged(int val)
{
	float prev = obs_source_get_balance_value(source);
	float bal = (float)val / 100.0f;

	if (abs(50 - val) < 10) {
		ui->balance->blockSignals(true);
		ui->balance->setValue(50);
		bal = 0.5f;
		ui->balance->blockSignals(false);
	}

	obs_source_set_balance_value(source, bal);

	auto undo_redo = [](const std::string &name, float val) {
		OBSSourceAutoRelease source =
			obs_get_source_by_name(name.c_str());
		obs_source_set_balance_value(source, val);
	};

	const char *name = obs_source_get_name(source);
	OBSBasic::Get()->undo_s.add_action(
		QTStr("Undo.Balance.Change").arg(name),
		std::bind(undo_redo, std::placeholders::_1, prev),
		std::bind(undo_redo, std::placeholders::_1, bal), name, name,
		true);
}

void OBSAdvAudioCtrl::ResetBalance()
{
	ui->balance->setValue(50);
}

void OBSAdvAudioCtrl::syncOffsetChanged(int milliseconds)
{
	int64_t prev = obs_source_get_sync_offset(source);
	int64_t val = int64_t(milliseconds) * NSEC_PER_MSEC;

	if (prev / NSEC_PER_MSEC == milliseconds)
		return;

	obs_source_set_sync_offset(source, val);

	auto undo_redo = [](const std::string &name, int64_t val) {
		OBSSourceAutoRelease source =
			obs_get_source_by_name(name.c_str());
		obs_source_set_sync_offset(source, val);
	};

	const char *name = obs_source_get_name(source);
	OBSBasic::Get()->undo_s.add_action(
		QTStr("Undo.SyncOffset.Change").arg(name),
		std::bind(undo_redo, std::placeholders::_1, prev),
		std::bind(undo_redo, std::placeholders::_1, val), name, name,
		true);
}

void OBSAdvAudioCtrl::monitoringTypeChanged(int index)
{
	obs_monitoring_type prev = obs_source_get_monitoring_type(source);

	obs_monitoring_type mt =
		(obs_monitoring_type)ui->monitoringType->itemData(index).toInt();
	obs_source_set_monitoring_type(source, mt);

	const char *type = nullptr;

	switch (mt) {
	case OBS_MONITORING_TYPE_NONE:
		type = "none";
		break;
	case OBS_MONITORING_TYPE_MONITOR_ONLY:
		type = "monitor only";
		break;
	case OBS_MONITORING_TYPE_MONITOR_AND_OUTPUT:
		type = "monitor and output";
		break;
	}

	const char *name = obs_source_get_name(source);
	blog(LOG_INFO, "User changed audio monitoring for source '%s' to: %s",
	     name ? name : "(null)", type);

	auto undo_redo = [](const std::string &name, obs_monitoring_type val) {
		OBSSourceAutoRelease source =
			obs_get_source_by_name(name.c_str());
		obs_source_set_monitoring_type(source, val);
	};

	OBSBasic::Get()->undo_s.add_action(
		QTStr("Undo.MonitoringType.Change").arg(name),
		std::bind(undo_redo, std::placeholders::_1, prev),
		std::bind(undo_redo, std::placeholders::_1, mt), name, name);
}

static inline void setMixer(obs_source_t *source, const int mixerIdx,
			    const bool checked)
{
	uint32_t mixers = obs_source_get_audio_mixers(source);
	uint32_t new_mixers = mixers;

	if (checked)
		new_mixers |= (1 << mixerIdx);
	else
		new_mixers &= ~(1 << mixerIdx);

	obs_source_set_audio_mixers(source, new_mixers);

	auto undo_redo = [](const std::string &name, uint32_t mixers) {
		OBSSourceAutoRelease source =
			obs_get_source_by_name(name.c_str());
		obs_source_set_audio_mixers(source, mixers);
	};

	const char *name = obs_source_get_name(source);
	OBSBasic::Get()->undo_s.add_action(
		QTStr("Undo.Mixers.Change").arg(name),
		std::bind(undo_redo, std::placeholders::_1, mixers),
		std::bind(undo_redo, std::placeholders::_1, new_mixers), name,
		name);
}

void OBSAdvAudioCtrl::mixer1Changed(bool checked)
{
	setMixer(source, 0, checked);
}

void OBSAdvAudioCtrl::mixer2Changed(bool checked)
{
	setMixer(source, 1, checked);
}

void OBSAdvAudioCtrl::mixer3Changed(bool checked)
{
	setMixer(source, 2, checked);
}

void OBSAdvAudioCtrl::mixer4Changed(bool checked)
{
	setMixer(source, 3, checked);
}

void OBSAdvAudioCtrl::mixer5Changed(bool checked)
{
	setMixer(source, 4, checked);
}

void OBSAdvAudioCtrl::mixer6Changed(bool checked)
{
	setMixer(source, 5, checked);
}

void OBSAdvAudioCtrl::SetVolumeWidget(VolumeType type)
{
	switch (type) {
	case VolumeType::Percent:
		ui->stackedWidget->setCurrentWidget(ui->percent);
		break;
	case VolumeType::dB:
		ui->stackedWidget->setCurrentWidget(ui->volume);
		break;
	}
}

void OBSAdvAudioCtrl::SetIconVisible(bool visible)
{
	visible ? ui->iconLabel->show() : ui->iconLabel->hide();
}
