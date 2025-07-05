#include <QApplication>
#include <QFileInfo>
#include <QFile>
#include <QTextStream>
#include "MainWindow.h"

//Includes for the codecs
#include "encoders/Base64Codec.h"
#include "encoders/Rot13.h"
#include "encoders/CaesarCipher.h"
#include "encoders/BinaryCodec.h"
#include "encoders/HexCodec.h"
#include "encoders/PigLatin.h"
#include "encoders/Atbash.h"
#include "encoders/MorseCodec.h"
#include "encoders/AESCodec.h"
#include "encoders/RSACodec.h"

QString useCodec(const QString &inputFile, const QString &outputFile,
                        const QString &function, const QString &codec,
                        const QString &key, const QString &shift) {
    QTextStream outlog(stdout);
    QFile inFile(inputFile);
    if (!inFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream err(stderr);
        err << "Error: Failed to open input file: " << inputFile << Qt::endl;
        return "Fail";
    }
    QTextStream inStream(&inFile);
    QString inputText = inStream.readAll();
    inFile.close();

    QString result;

    if (codec == "base64") {
        result = (function == "encode") ? Base64Codec::transform(inputText, false) : Base64Codec::transform(inputText, true);
    } else if (codec == "rot13") {
        result = Rot13::transform(inputText);
    } else if (codec == "caesar") {
        if (!shift.isEmpty()) {
            result = (function == "encode") ? CaesarCipher::transform(inputText, shift.toInt(), false) : CaesarCipher::transform(inputText, shift.toInt(), true);
        } else {
            QTextStream err(stderr);
            err << "Error: Missing shift amount." << Qt::endl;
            return "Fail";
        }
    } else if (codec == "binary") {
        result = (function == "encode") ? BinaryCodec::transform(inputText, false) : BinaryCodec::transform(inputText, true);
    } else if (codec == "hex") {
        result = (function == "encode") ? HexCodec::transform(inputText, false) : HexCodec::transform(inputText, true);
    } else if (codec == "piglatin") {
        result = (function == "encode") ? PigLatin::transform(inputText, false) : PigLatin::transform(inputText, true);
    } else if (codec == "atbash") {
        result = Atbash::transform(inputText);
    } else if (codec == "morse") {
        result = (function == "encode") ? MorseCodec::transform(inputText, false) : MorseCodec::transform(inputText, true);
    } else if (codec == "aes") {
        if (!key.isEmpty()) {
            result = (function == "encode") ? AESCodec::encode(inputText, key) : AESCodec::decode(inputText, key);
        } else {
            QTextStream err(stderr);
            err << "Error: Missing Key." << Qt::endl;
            return "Fail";
        }
    } else if (codec == "rsa") {
        if (!key.isEmpty()) {
            result = (function == "encode") ? RSACodec::encode(inputText.toUtf8(), key) : RSACodec::decode(inputText.toUtf8(), key);
        } else {
            QTextStream err(stderr);
            err << "Error: Missing Key." << Qt::endl;
            return "Fail";
        }
    } else {
        QTextStream err(stderr);
        err << "Error: Unknown codec: " << codec << Qt::endl;
        return "Fail";
    }

    if (!outputFile.isEmpty()) {
        QFile outFile(outputFile);
        if (!outFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
                    QTextStream err(stderr);
                    err << "Error: Failed to open output file: " << outputFile << Qt::endl;
                    return "Fail";
                }
        QTextStream outStream(&outFile);
        outStream << result;
        outFile.close();
    } else {
        outlog << result << Qt::endl;
    }
    return "Success";
}

void printUsage() {
    QTextStream out(stdout);
    out << "Usage:\n"
        << "  decode -i <input_file> -o <output_file> -f <encode|decode> -c <codec>\n\n -k <key>"
        << "Available codecs: base64, rot13, caesar, binary, hex, piglatin, atbash, morse, aes, rsa\n";
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/decode.svg"));
    MainWindow window;

    QStringList args = QCoreApplication::arguments();
    args.removeFirst();

    QTextStream out(stdout);

    QString inputFile, outputFile, function, codec, key, shift;
    bool headless = false;

    for (int i = 0; i < args.size(); ++i) {
        QString arg = args[i];
        if ((arg == "-i" || arg == "--input") && i + 1 < args.size()){
            inputFile = args[++i];
            headless=true;
        } else if ((arg == "-o" || arg == "--output") && i + 1 < args.size()) {
            outputFile = args[++i];
            headless=true;
        } else if ((arg == "-f" || arg == "--function") && i + 1 < args.size()) {
            function = args[++i];
            headless=true;
        } else if ((arg == "-c" || arg == "--codec") && i + 1 < args.size()) {
            codec = args[++i];
            headless=true;
        } else if ((arg == "-k" || arg == "--key") && i + 1 < args.size()) {
            key = args[++i];
            headless=true;
        } else if ((arg == "-s" || arg == "--shift") && i + 1 < args.size()) {
            shift = args[++i];
            headless=true;
        } else if (arg == "-h" || arg == "--help") {
            headless=true;
            printUsage();
            return 0;
        } else if (arg == "--clearsession") {
            out << "Clear Session.\n";
            window.clearSession();
            return app.exec();
        } else {
            out << "Not a recognized flag recognized flag.: " << arg << Qt::endl;
            if (QFileInfo::exists(arg) && QFileInfo(arg).isFile()) {
                out << "URL recognized: " << arg << Qt::endl;
                window.openFile(arg);
            }
        }
    }
    if (headless==false){
        window.show();
    }
    if (!inputFile.isEmpty() && (function == "encode" || function == "decode") && !codec.isEmpty())      {
        QString status = useCodec(inputFile, outputFile, function, codec, key, shift);
        if (status != "Success") {
            qCritical() << status;
            return 1;
        }
        return 0;
    }
    return app.exec();
}
