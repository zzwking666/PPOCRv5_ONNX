#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_BagsRealTimeDisplay.h"
#include <QTimer>
#include "rqw_LabelClickable.h"
#include "DlgCloseForm.h"
#include "ConfigModule.hpp"
#include "CameraModule.hpp"
#include "DlgProductSet.h"
#include "PanZoomLabel.h"


QT_BEGIN_NAMESPACE
namespace Ui { class BagsRealTimeDisplayClass; };
QT_END_NAMESPACE

class BagsRealTimeDisplay : public QMainWindow
{
    Q_OBJECT

public:
    BagsRealTimeDisplay(ConfigModule& configModule,CameraModule& cameraModule,QWidget* parent = nullptr);
    ~BagsRealTimeDisplay();
public:
	void initializeComponents();
public:
	void build_ui();
	void build_connect();
	void build_BagsRealTimeDisplayData();
    void build_DlgCloseForm();
	void build_DlgProductSet();
	void build_PanZoomLabel();
protected:
	bool eventFilter(QObject* watched, QEvent* event) override;
private:
	void touchManualViewOperation();
public slots:
	void updateCameraLabelState(int cameraIndex, bool state);

	void onUpdateStatisticalInfoUI();

    void onCameraDisplay(size_t index, const QImage& image);

	void setConfigWindowClosed();
private slots:
	void pbtn_exit_clicked();
	void pbtn_set_clicked();
	void btn_jianshaobaoguang1_clicked();
	void btn_zengjiabaoguang1_clicked();
	void btn_jianshaobaoguang2_clicked();
	void btn_zengjiabaoguang2_clicked();
	void pbtn_resetProduct_clicked();

	void cbb_qiehuanxianshi_currentIndexChanged(int index);

	void lb_title_clicked();
public:
	DlgProductSet* _dlgProductSet = nullptr;
private:
	rw::rqw::ClickableLabel* clickableTitle = nullptr;
	int minimizeCount{ 3 };
	DlgCloseForm* _dlgCloseForm = nullptr;

	// 正面背面切换锚点
	int lastCameraCaptureCount{ 0 };
	int lastCameraCaptureIndex{ 1 };

	// 自定义可操作label
	PanZoomLabel* _panZoomLabel = nullptr;
	PanZoomLabel::ViewState _frontViewState{};
	PanZoomLabel::ViewState _backViewState{};
	int _currentViewCamera{ 0 };	// 1:正面相机, 2:背面相机

	// 手动操作后冻结图像更新
	bool _freezeImageUpdate{ false };
	int _freezeAfterManualMs{ 3000 };
	QTimer* _manualFreezeTimer{ nullptr };
private:
    Ui::BagsRealTimeDisplayClass* ui;
    ConfigModule& _configModule;
	CameraModule& _cameraModule;
};

