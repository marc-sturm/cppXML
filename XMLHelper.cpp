#include "XmlHelper.h"
#include "Helper.h"

#include <QUrl>
#include <QTemporaryFile>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QTextStream>

#include <iostream>
#include <libxml/parser.h>
#include <libxml/xmlschemas.h>

#include <QScopedPointer>
#include <QTemporaryFile>
#include <QUrl>
#include <QDomDocument>
#include <QRegularExpression>

QString XmlHelper::isValidXml(QString xml_file)
{
    QFile file(xml_file);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return QString("Error while opening the file '%1'").arg(xml_file);
    }

	//special handling for HTML
    if (xml_file.endsWith("html", Qt::CaseInsensitive))
    {
        QTextStream stream(&file);
        QString html_content = stream.readAll();
        file.close();
        QRegularExpression regex("&[^;]+;"); // ignore special HTML characters
        QByteArray processed = html_content.replace(regex, "").toLatin1();
        #if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
        QDomDocument doc;
        QDomDocument::ParseOptions options = QDomDocument::ParseOption::Default;
        QDomDocument::ParseResult result = doc.setContent(processed, options);

        if (!result.errorMessage.isEmpty())
        {
            return "Error parsing HTML at line " + QString::number(result.errorLine) + " column " + QString::number(result.errorColumn) + ": " + result.errorMessage;
        }

        QDomElement root = doc.documentElement();
        if (root.isNull())
        {
            return "Invalid HTML: missing root element";
        }

        return "";
        #else
        QDomDocument doc;
        QString error_message;
        int error_line = 0, error_column = 0;

        // Use the standard setContent() method
        if (!doc.setContent(processed, &error_message, &error_line, &error_column))
        {
            file.close();
            return "Error parsing HTML at line " + QString::number(error_line) + ", column " + QString::number(error_column)+ ": " + error_message;
        }
        else
        {
            return "";
        }
        #endif
    }

    QXmlStreamReader xmlReader(&file);
    while (!xmlReader.atEnd() && !xmlReader.hasError())
    {
        xmlReader.readNext();
    }

    if (xmlReader.hasError())
    {
        file.close();
		return QString("XML parsing error: %1 at line %2, column %3")
        .arg(xmlReader.errorString())
            .arg(xmlReader.lineNumber())
            .arg(xmlReader.columnNumber());
    }
    file.close();
    return "";
}

QString XmlHelper::isValidXml(QString xml_name, QString schema_file)
{
    // create schema url (both for native files and files from resources)
    QUrl schema_url;
    QScopedPointer<QTemporaryFile> tmp_file(QTemporaryFile::createNativeFile(schema_file));
    if (!tmp_file.isNull()) {
        schema_url = QUrl::fromLocalFile(tmp_file->fileName());
    } else {
        schema_url = QUrl::fromLocalFile(schema_file);
    }

    // load the schema
    xmlSchemaParserCtxtPtr schemaParserCtxt = xmlSchemaNewParserCtxt(schema_url.toLocalFile().toUtf8().constData());
    if (!schemaParserCtxt) {
        return "Failed to create schema parser context.";
    }

    xmlSchemaPtr schema = xmlSchemaParse(schemaParserCtxt);
    xmlSchemaFreeParserCtxt(schemaParserCtxt);

    if (!schema) {
        return "Failed to parse XSD schema: " + schema_url.toString();
    }

    // create validation context
    xmlSchemaValidCtxtPtr schemaValidCtxt = xmlSchemaNewValidCtxt(schema);
    if (!schemaValidCtxt) {
        xmlSchemaFree(schema);
        return "Failed to create schema validation context.";
    }

    // load the XML file
    xmlDocPtr xmlDoc = xmlReadFile(xml_name.toUtf8().constData(), NULL, XML_PARSE_NONET);
    if (!xmlDoc) {
        xmlSchemaFreeValidCtxt(schemaValidCtxt);
        xmlSchemaFree(schema);
        return "Failed to parse XML file: " + xml_name;
    }

    // validate XML
    int validationResult = xmlSchemaValidateDoc(schemaValidCtxt, xmlDoc);

    // cleanup
    xmlFreeDoc(xmlDoc);
    xmlSchemaFreeValidCtxt(schemaValidCtxt);
    xmlSchemaFree(schema);

    if (validationResult == 0) {
        return ""; // validation successful, no errors found
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
