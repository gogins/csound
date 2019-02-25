/*
    Copyright (C) 2008-2016 Andres Cabrera
    mantaraya36@gmail.com

    This file is part of CsoundQt.

    CsoundQt is free software; you can redistribute it
    and/or modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    CsoundQt is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with Csound; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
    02111-1307 USA
*/

#ifndef qcsound_H
#define qcsound_H

#include <QObject>
#include <QDebug>
#include <csoundwebview.h>
#include <csound.h>

class CsoundWebView;
namespace csound {
    class CsoundProducer;
}

class QCsound : public QObject
{
    Q_OBJECT
public:
    explicit QCsound(QObject *parent = 0);
    virtual ~QCsound();
    void setMessageCallback(void (*messageCallback)(CSOUND *csound, int level, const char *format, va_list valist));
    void SetMessageCallback(void (*messageCallback)(CSOUND *csound, int level, const char *format, va_list valist));
    CsoundWebView *csound_web_view;
public slots:
    ///void registerConsole(ConsoleWidget *console);
    int cleanup();
    int compileCsd(const QString &filename);
    int compileCsdText(const QString &text);
    int compileOrc(const QString &text);
    double evalCode(const QString &text);
    double get0dBFS();
    int getApiVersion();
    double getControlChannel(const QString &name);
    CSOUND *getCsound();
    qint64 getCurrentTimeSamples();
    QString getEnv(const QString &name);
    int getKsmps();
    int getNchnls();
    int getNchnlsInput();
    QString getOutputName();
    double getScoreOffsetSeconds();
    double getScoreTime();
    int getSr();
    QString getStringChannel(const QString &name);
    int getVersion();
    bool isPlaying();
    int isScorePending();
    void message(const QString &text);
    int perform();
    int performKsmps();
    int readScore(const QString &text);
    void reset();
    void rewindScore();
    int runUtility(const QString &command, int argc, char **argv);
    int scoreEvent(char type, const double *pFields, long numFields);
    void setControlChannel(const QString &name, double value);
    int setGlobalEnv(const QString &name, const QString &value);
    void setHostData(void *host_data);
    void setMessageCallback(QObject *callback);
    void setInput(const QString &name);
    int setOption(const QString &name);
    void setOutput(const QString &name, const QString &type, const QString &format);
    void setScoreOffsetSeconds(double value);
    void setScorePending(bool value);
    void setStringChannel(const QString &name, const QString &value);
    int start();
    void stop();
    double tableGet(int table_number, int index);
    int tableLength(int table_number);
    void tableSet(int table_number, int index, double value);
    void setMetadata(const QString &key, const QString &value);
    QString getMetadata(const QString &key) const;
    void setDoGitCommit(bool do_git_commit);
    bool getDoGitCommit() const;
    int performAndPostProcess();

    int Cleanup();
    int CompileCsd(const QString &filename);
    int CompileCsdText(const QString &text);
    int CompileOrc(const QString &text);
    double EvalCode(const QString &text);
    double Get0dBFS();
    int GetApiVersion();
    double GetControlChannel(const QString &name);
    CSOUND *GetCsound();
    qint64 GetCurrentTimeSamples();
    QString GetEnv(const QString &name);
    int GetKsmps();
    int GetNchnls();
    int GetNchnlsInput();
    QString GetOutputName();
    double GetScoreOffsetSeconds();
    double GetScoreTime();
    int GetSr();
    QString GetStringChannel(const QString &name);
    int GetVersion();
    bool IsPlaying();
    int IsScorePending();
    void Message(const QString &text);
    int Perform();
    int PerformKsmps();
    int ReadScore(const QString &text);
    void Reset();
    void RewindScore();
    int RunUtility(const QString &command, int argc, char **argv);
    int ScoreEvent(char type, const double *pFields, long numFields);
    void SetControlChannel(const QString &name, double value);
    int SetGlobalEnv(const QString &name, const QString &value);
    void SetHostData(void *host_data);
    void SetMessageCallback(QObject *callback);
    void SetInput(const QString &name);
    int SetOption(const QString &name);
    void SetOutput(const QString &name, const QString &type, const QString &format);
    void SetScoreOffsetSeconds(double value);
    void SetScorePending(bool value);
    void SetStringChannel(const QString &name, const QString &value);
    int Start();
    void Stop();
    double TableGet(int table_number, int index);
    int TableLength(int table_number);
    void TableSet(int table_number, int index, double value);
    void SetMetadata(const QString &key, const QString &value);
    QString GetMetadata(const QString &key) const;
    void SetDoGitCommit(bool do_git_commit);
    bool GetDoGitCommit() const;
    int PerformAndPostProcess();
signals:
    void passMessages(QString message);
private:
    static void csoundMessageCallback_(CSOUND *csound,
                                             int attributes,
                                             const char *format,
                                             va_list args);
    void csoundMessageCallback(int attributes,
                               const char *format,
                               va_list args);
    QObject *message_callback;
    csound::CsoundProducer *csound;
    QString csound_messages_buffer;
};

#endif // CsoundHtmlWrapper_H
