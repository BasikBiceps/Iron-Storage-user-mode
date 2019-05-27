#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include <QQuickStyle>
#include <QQmlContext>

#include <QDebug>

#include "qmlfacade.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);
    QQuickStyle::setStyle("Universal");

    QmlFacade qmlFacade;
    QQmlApplicationEngine engine;

    qmlRegisterType<QmlFacade>("backend", 1, 0, "QmlFacade");
    engine.rootContext()->setContextProperty("qmlFacadeContext", &qmlFacade);

    QUrl const url(QStringLiteral("qrc:/main.qml"));
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
