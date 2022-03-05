#pragma once

#include <obs.hpp>
#include <QWidget>
#include <QPointer>
#include <QDoubleSpinBox>
#include <QStackedWidget>
#include "balance-slider.hpp"

#include "ui_OBSAdvAudioCtrl.h"

class QLabel;
class QSpinBox;
class QCheckBox;
class QComboBox;

enum class VolumeType {
	dB,
	Percent,
};

class OBSAdvAudioCtrl : public QWidget {
	Q_OBJECT

	std::unique_ptr<Ui::OBSAdvAudioCtrl> ui;

private:
	OBSSource source;

	OBSSignal volChangedSignal;
	OBSSignal syncOffsetSignal;
	OBSSignal flagsSignal;
	OBSSignal monitoringTypeSignal;
	OBSSignal mixersSignal;
	OBSSignal activateSignal;
	OBSSignal deactivateSignal;
	OBSSignal balChangedSignal;

	static void OBSSourceActivated(void *param, calldata_t *calldata);
	static void OBSSourceDeactivated(void *param, calldata_t *calldata);
	static void OBSSourceFlagsChanged(void *param, calldata_t *calldata);
	static void OBSSourceVolumeChanged(void *param, calldata_t *calldata);
	static void OBSSourceSyncChanged(void *param, calldata_t *calldata);
	static void OBSSourceMonitoringTypeChanged(void *param,
						   calldata_t *calldata);
	static void OBSSourceMixersChanged(void *param, calldata_t *calldata);
	static void OBSSourceBalanceChanged(void *param, calldata_t *calldata);

public:
	OBSAdvAudioCtrl(QWidget *parent, obs_source_t *source_);

	inline obs_source_t *GetSource() const { return source; }

	void SetVolumeWidget(VolumeType type);
	void SetIconVisible(bool visible);

public slots:
	void SourceActiveChanged(bool active);
	void SourceFlagsChanged(uint32_t flags);
	void SourceVolumeChanged(float volume);
	void SourceSyncChanged(int64_t offset);
	void SourceMonitoringTypeChanged(int type);
	void SourceMixersChanged(uint32_t mixers);
	void SourceBalanceChanged(int balance);

	void volumeChanged(double db);
	void percentChanged(int percent);
	void downmixMonoChanged(bool checked);
	void balanceChanged(int val);
	void syncOffsetChanged(int milliseconds);
	void monitoringTypeChanged(int index);
	void mixer1Changed(bool checked);
	void mixer2Changed(bool checked);
	void mixer3Changed(bool checked);
	void mixer4Changed(bool checked);
	void mixer5Changed(bool checked);
	void mixer6Changed(bool checked);
	void ResetBalance();
};
