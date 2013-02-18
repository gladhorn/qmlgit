
#include <QQuickTextDocument>
#include <QSyntaxHighlighter>

#include <qdebug.h>

class Highlighter : public QSyntaxHighlighter
{
public:
    Highlighter(QTextDocument *parent)
        : QSyntaxHighlighter(parent)
    {}

    void highlightBlock ( const QString & text )
    {
        if (text.isEmpty())
            return;

        QTextCharFormat diffRemoved;
        diffRemoved.setFontWeight(QFont::Bold);
        diffRemoved.setForeground(Qt::darkMagenta);


        QTextCharFormat diffAdded;
        diffAdded.setFontWeight(QFont::Bold);
        diffAdded.setForeground(Qt::darkGreen);


        if (text.at(0) == QLatin1Char('-')) {
            setFormat(0, text.length(), diffRemoved);
        } else if (text.at(0) == QLatin1Char('+')) {
            setFormat(0, text.length(), diffAdded);
        }
    }
};

class DiffHighlighter : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(QQuickItem *target READ target WRITE setTarget NOTIFY targetChanged)

public:
    DiffHighlighter() : m_highlighter(0)
    {}

    QQuickItem *target() { return m_target; }

    void setTarget(QQuickItem *target)
    {
        qDebug() << "set target " << m_target;
        m_target = target;
        emit targetChanged();

        QVariant doc = m_target->property("textDocument");
        if (doc.canConvert<QQuickTextDocument*>()) {
            QQuickTextDocument *qqDoc = doc.value<QQuickTextDocument*>();

            if (qqDoc) {
                qDebug() << "got text doc: ";

                if (m_highlighter)
                    delete m_highlighter;
                m_highlighter = new Highlighter(qqDoc->textDocument());
            }
        }
    }

Q_SIGNALS:
    void targetChanged();

private:
    QQuickItem *m_target;
    QSyntaxHighlighter *m_highlighter;
};