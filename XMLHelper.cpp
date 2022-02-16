#include "XmlHelper.h"
#include "Exceptions.h"
#include "Helper.h"
#include <QXmlSchemaValidator>
#include <QXmlSchema>
#include <QUrl>
#include <QTemporaryFile>
#include <QXmlSimpleReader>
#include <QXmlInputSource>

QString XmlHelper::isValidXml(QString xml_file)
{
	QXmlSimpleReader xmlReader;

	XmlValidationMessageHandler2 handler;
	xmlReader.setContentHandler(&handler);
	xmlReader.setErrorHandler(&handler);

	QFile file(xml_file);
	QXmlInputSource source(&file);

	if (!xmlReader.parse(&source))
	{
		return handler.errorString();
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

QString XmlHelper::XmlValidationMessageHandler::messages()
{
	return messages_;
}

void XmlHelper::XmlValidationMessageHandler::handleMessage(QtMsgType /*type*/, const QString& description, const QUrl& /*identifier*/, const QSourceLocation& /*sourceLocation*/)
{
	messages_ = messages_ + description + " ";
}
