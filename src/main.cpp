#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include <QQuickStyle>
#include <QQmlContext>
#include <QProcess>

#include <QDebug>

#include "qmlfacade.h"
#include "diskmanager.h"
#include "diskinfomodel.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);
    QQuickStyle::setStyle("Universal");

    QProcess process;
    QString file = qApp->applicationDirPath() + "/driverLoader(x64)/IronStorageDriverLoader.exe";
    process.start(file);

    if (!process.waitForStarted(-1))
    {
        qFatal("Cannot run driver loader");
    }
    process.waitForFinished(-1);

    if (process.exitCode() != 0)
    {
        qFatal(qUtf8Printable(QString(process.readAllStandardError())));
    }

    DiskManager diskManager;
    DiskInfoModel diskInfoModel;
    QmlFacade qmlFacade;
    QQmlApplicationEngine engine;

    QObject::connect(&qmlFacade, &QmlFacade::languageChanged, &engine, &QQmlEngine::retranslate);
    qmlFacade.setDiskServices(&diskManager, &diskInfoModel);

    qmlRegisterType<QmlFacade>("backend", 1, 0, "QmlFacade");
    engine.rootContext()->setContextProperty("qmlFacadeContext", &qmlFacade);

    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
        {
            QCoreApplication::exit(-1);
        }
    }, Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
