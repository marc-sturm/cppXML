#include "XmlHelper.h"
#include "Helper.h"
#include <QXmlSchemaValidator>
#include <QXmlSchema>
#include <QUrl>
#include <QTemporaryFile>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

QString XmlHelper::isValidXml(QString xml_file)
{
    QFile file(xml_file);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return QString("Error while opening the file '%1'").arg(xml_file);
    }

    QXmlStreamReader xmlReader(&file);
    while (!xmlReader.atEnd() && !xmlReader.hasError())
    {
        xmlReader.readNext();
    }

    if (xmlReader.hasError())
    {
        return QString("XML Parsing Error: %1 at line %2, column %3")
        .arg(xmlReader.errorString())
            .arg(xmlReader.lineNumber())
            .arg(xmlReader.columnNumber());
    }

    return "";
}

QString XmlHelper::isValidXml(QString xml_name, QString schema_file)
{
	//create schema url (both for native files and files from resources)
	QUrl schema_url;
	QScopedPointer<QTemporaryFile> tmp_file(QTemporaryFile::createNativeFile(schema_file));
	if (!tmp_file.isNull())
	{
		schema_url = QUrl::fromLocalFile(tmp_file->fileName());
	}
	else
	{
		schema_url = QUrl::fromLocalFile(schema_file);
	}

	//load schema
	QXmlSchema schema;
	if (!schema.load(schema_url))
	{
		THROW(FileParseException, "XML schema '" + schema_url.toString() + "'  is not valid/present.");
	}

	//validate file
	QXmlSchemaValidator validator(schema);
	XmlValidationMessageHandler handler;
	validator.setMessageHandler(&handler);
	QSharedPointer<QFile> xml_file = Helper::openFileForReading(xml_name);
	if (validator.validate(xml_file.data(), schema_url))
	{
		return "";
	}

	return handler.messages();
}

QString XmlHelper::format(QString xml)
{
	QString xml_out;
	QXmlStreamReader reader(xml);
	QXmlStreamWriter writer(&xml_out);
	writer.setAutoFormatting(true);

	while (!reader.atEnd())
	{
		reader.readNext();
		if (!reader.isWhitespace())
		{
			writer.writeCurrentToken(reader);
		}
	}

	return xml_out;
}

QString XmlHelper::XmlValidationMessageHandler::messages()
{
	return messages_;
}

void XmlHelper::XmlValidationMessageHandler::handleMessage(QtMsgType /*type*/, const QString& description, const QUrl& /*identifier*/, const QSourceLocation& /*sourceLocation*/)
{
	messages_ = messages_ + description + " ";
}
