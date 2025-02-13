#ifndef XMLHELPER_H
#define XMLHELPER_H

#include "cppXML_global.h"
#include <QAbstractMessageHandler>

///Helper class for XML handling.
class CPPXMLSHARED_EXPORT XmlHelper
{
public:
    ///Returns an empty string if the xml file is well-formed, or the error message otherwise. Throws an Exception if the xml file is not ok.
    static QString isValidXml(QString xml_file);

    ///Returns an empty string if the xml file validates against the schema, or the error message otherwise. Throws an Exception if the schema/xml file are not ok.
    static QString isValidXml(QString xml_file, QString schema_file);

	///Format XML to make it human-readable
	static QString format(QString xml);

private:
    ///XML validation message handler with schema.
    class XmlValidationMessageHandler
                    : public QAbstractMessageHandler
    {
    public:
            QString messages();

    protected:
            void handleMessage(QtMsgType type, const QString& description, const QUrl& identifier, const QSourceLocation& sourceLocation);
            QString messages_;
    };

    //declared away
    XmlHelper() = delete;
};

#endif
