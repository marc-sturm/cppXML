#include "XmlHelper.h"
#include "Helper.h"
#include "VersatileFile.h"
#include <QUrl>
#include <QTemporaryFile>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QTextStream>
#include <QScopedPointer>
#include <QRegularExpression>
#include <libxml/parser.h>
#include <libxml/xmlschemas.h>

QString XmlHelper::isValidXml(QString xml_file)
{
	//special handling for HTML
    if (xml_file.endsWith("html", Qt::CaseInsensitive))
    {
        //load HTML into string
        VersatileFile file(xml_file);
        file.open(QFile::ReadOnly|QFile::Text);
        QString html_content = file.readAll();

        //ignore special HTML characters
        QRegularExpression regex("&[^;]+;");
        QByteArray processed = html_content.replace(regex, "").toLatin1();

        //parse
        try
        {
            parse(processed);
        }
        catch (Exception& e)
        {
            return e.message();
        }
    }
    else
    {
        QFile file(xml_file);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            return QString("Error while opening XML file '%1'").arg(xml_file);
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
    }

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

QDomDocument XmlHelper::load(QString xml_file)
{
    VersatileFile file(xml_file);
    file.open(QFile::ReadOnly|QFile::Text);
    return parse(file.readAll());
}

QDomDocument XmlHelper::parse(QByteArray xml_string)
{
    QDomDocument doc;

    //parse
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
    QDomDocument::ParseResult result = doc.setContent(xml_string);
    if (!result.errorMessage.isEmpty())
    {
        THROW(ArgumentException, "Error parsing XML at line " + QString::number(result.errorLine) + " column " + QString::number(result.errorColumn) + ": " + result.errorMessage);
    }
#else
    QString error_message;
    int error_line = 0;
    int error_column = 0;
    if (!doc.setContent(xml_string, &error_message, &error_line, &error_column))
    {
        THROW(ArgumentException, "Error parsing XML at line " + QString::number(error_line) + " column " + QString::number(error_column) + ": " + error_message);
    }
#endif

    //check for root element
    QDomElement root = doc.documentElement();
    if (root.isNull())
    {
        THROW(ArgumentException, "Invalid XML: missing root element");
    }

    return doc;
}
