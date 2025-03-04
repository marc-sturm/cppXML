#include "XmlHelper.h"
#include "Helper.h"
// #include <QXmlSchemaValidator>
// #include <QXmlSchema>
#include <QUrl>
#include <QTemporaryFile>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QDomDocument>

#include <iostream>
#include <libxml/parser.h>
#include <libxml/xmlschemas.h>

#include <QScopedPointer>
#include <QTemporaryFile>
#include <QUrl>

QString XmlHelper::isValidXml(QString xml_file)
{
    QFile file(xml_file);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return QString("Error while opening the file '%1'").arg(xml_file);
    }

    if (xml_file.endsWith("html", Qt::CaseInsensitive))
    {
        QDomDocument doc;
        if (!doc.setContent(&file))
        {
            return "Failed to parse HTML as XML";
        } else
        {
           return "";
        }
        file.close();
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
    // //create schema url (both for native files and files from resources)
    // QUrl schema_url;
    // QScopedPointer<QTemporaryFile> tmp_file(QTemporaryFile::createNativeFile(schema_file));
    // if (!tmp_file.isNull())
    // {
    // 	schema_url = QUrl::fromLocalFile(tmp_file->fileName());
    // }
    // else
    // {
    // 	schema_url = QUrl::fromLocalFile(schema_file);
    // }

    // //load schema
    // QXmlSchema schema;
    // if (!schema.load(schema_url))
    // {
    // 	THROW(FileParseException, "XML schema '" + schema_url.toString() + "'  is not valid/present.");
    // }

    // //validate file
    // QXmlSchemaValidator validator(schema);
    // XmlValidationMessageHandler handler;
    // validator.setMessageHandler(&handler);
    // QSharedPointer<QFile> xml_file = Helper::openFileForReading(xml_name);
    // if (validator.validate(xml_file.data(), schema_url))
    // {
    // 	return "";
    // }

    // return handler.messages();




    //create schema url (both for native files and files from resources)
    QUrl schema_url;
    QScopedPointer<QTemporaryFile> tmp_file(QTemporaryFile::createNativeFile(schema_file));
    if (!tmp_file.isNull()) {
        schema_url = QUrl::fromLocalFile(tmp_file->fileName());
    } else {
        schema_url = QUrl::fromLocalFile(schema_file);
    }

    // Load the schema
    xmlSchemaParserCtxtPtr schemaParserCtxt = xmlSchemaNewParserCtxt(schema_url.toLocalFile().toUtf8().constData());
    if (!schemaParserCtxt) {
        return "Failed to create schema parser context.";
    }

    xmlSchemaPtr schema = xmlSchemaParse(schemaParserCtxt);
    xmlSchemaFreeParserCtxt(schemaParserCtxt);

    if (!schema) {
        return "Failed to parse XSD schema: " + schema_url.toString();
    }

    // Create validation context
    xmlSchemaValidCtxtPtr schemaValidCtxt = xmlSchemaNewValidCtxt(schema);
    if (!schemaValidCtxt) {
        xmlSchemaFree(schema);
        return "Failed to create schema validation context.";
    }

    // Load the XML file
    xmlDocPtr xmlDoc = xmlReadFile(xml_name.toUtf8().constData(), NULL, XML_PARSE_NONET);
    if (!xmlDoc) {
        xmlSchemaFreeValidCtxt(schemaValidCtxt);
        xmlSchemaFree(schema);
        return "Failed to parse XML file: " + xml_name;
    }

    // Validate XML
    int validationResult = xmlSchemaValidateDoc(schemaValidCtxt, xmlDoc);

    // Cleanup
    xmlFreeDoc(xmlDoc);
    xmlSchemaFreeValidCtxt(schemaValidCtxt);
    xmlSchemaFree(schema);

    if (validationResult == 0) {
        return ""; // Validation successful
    } else {
        return "XML validation failed against schema: " + schema_url.toString();
    }
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

// QString XmlHelper::XmlValidationMessageHandler::messages()
// {
// 	return messages_;
// }

// void XmlHelper::XmlValidationMessageHandler::handleMessage(QtMsgType /*type*/, const QString& description, const QUrl& /*identifier*/, const QSourceLocation& /*sourceLocation*/)
// {
// 	messages_ = messages_ + description + " ";
// }
