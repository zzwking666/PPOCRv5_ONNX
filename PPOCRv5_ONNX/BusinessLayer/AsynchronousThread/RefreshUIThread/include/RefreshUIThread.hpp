#pragma once
#include <QThread>

class RefreshUIThread : public QThread
{
	Q_OBJECT
public:
	explicit RefreshUIThread(QObject* parent = nullptr);
	~RefreshUIThread() override;

	void startThread();

	void stopThread();
protected:
	void run() override;
signals:
	void emit_RefreshUI();
private:
	std::atomic<bool> running{ false };
};
