#pragma once

#include <QDialog>
#include "ConfigModule.hpp"
#include "CameraModule.hpp"


QT_BEGIN_NAMESPACE
namespace Ui { class DlgProductSetClass; };
QT_END_NAMESPACE

class DlgProductSet : public QDialog
{
	Q_OBJECT

public:
	DlgProductSet(ConfigModule& configModule, CameraModule& cameraModule, QWidget* parent = nullptr);
	~DlgProductSet();

public:
	void build_ui();
	void read_config();
	void build_connect();

protected:
	void showEvent(QShowEvent* event) override;

signals:
	void paramsChanged();

	void emit_changeLanguage(int index);
public slots:
	void onUpdateFrameLost();
private slots:
	void btn_close_clicked();

	void btn_chengfaqi1_clicked();
	void btn_houfenpin1_clicked();
	void btn_zengyi1_clicked();
	void btn_baoguang1_clicked();

	void btn_chengfaqi2_clicked();
	void btn_houfenpin2_clicked();
	void btn_zengyi2_clicked();
	void btn_baoguang2_clicked();

	void btn_xuantingshijian_clicked();
	void btn_qiehuanzhangshu_clicked();
	void btn_youyijuli_clicked();
	void btn_suofang_clicked();
	void ckb_jingxiang_toggled(bool checked);
public:
	Ui::DlgProductSetClass* ui;
private:
	ConfigModule& _configModule;
	CameraModule& _cameraModule;
};

