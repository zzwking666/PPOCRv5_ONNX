#include <QFile>

#include <QtWidgets/QApplication>
#include "rqwu/rqwu_core.h"
#include "AppRuntime.hpp"
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    Q_INIT_RESOURCE(BagsRealTimeDisplay);
    rw::rqwu::ini();

	AppRuntime appRuntime;
    if (!appRuntime.initialize())
    {
        return 1;
    }

    appRuntime.show();

    QObject::connect(&app, &QCoreApplication::aboutToQuit,
        [&appRuntime]()
        {
	        appRuntime.shutdown();
        }
    );
  
    return app.exec();
}
