#pragma once

#include <ESPAsyncWebServer.h>
#include "generated/dash.h"
#include <TaskSchedulerDeclarations.h>

class IDashboard {
public:
	virtual DashboardData getDashboardData() = 0;

	virtual DashboardData *getMutualDashboardData() = 0;

	virtual void addScheduler(Scheduler *scheduler) = 0;
};

class Dashboard : public IDashboard {
private:
	const unsigned long defaultSendRepeatInterval = 1000;

	void send();

	unsigned long sendRepeatInterval = defaultSendRepeatInterval;
	Task *tLoop = nullptr;
	DashboardData dashboardData{};

public:
	Dashboard() = default;

	DashboardData getDashboardData() override { return dashboardData; }

	DashboardData *getMutualDashboardData() override { return &dashboardData; }

	void addScheduler(Scheduler *scheduler) override;

	void setSendRepeatInterval(unsigned long sendRepeatInterval);
};

IDashboard *getDashboard();
