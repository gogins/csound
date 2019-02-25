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

    You should have received a copy o    ScoreEvent *event = 0;
    while (!csound_event_queue.try_dequeue(event)) {
        delete event;
    }
    char *score_text = 0;
    while (!csound_score_queue.try_dequeue(score_text)) {
        free(score_text);
    }
f the GNU Lesser General Public
    License along with Csound; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
    02111-1307 USA
*/

#include <CsoundProducer.hpp>

#include "qcsound.h"
#include <QApplication>
#include <QDebug>

QCsound::QCsound(QObject *parent) :
    QObject(parent),
    csound_web_view(nullptr),
  message_callback(nullptr)
{
    csound = new csound::CsoundProducer();
    csound->SetHostData(this);
    csound->SetMessageCallback(QCsound::csoundMessageCallback_);
}

QCsound::~QCsound() {
    delete csound;
}

int QCsound::cleanup() {
    return csound->Cleanup();
}

int QCsound::performAndPostProcess() {
    return csound->PerformAndPostProcess();
}

int QCsound::compileCsd(const QString &filename) {
    return csound->CompileCsd(filename.toLocal8Bit());
}

int QCsound::compileCsdText(const QString &text) {
    return csound->CompileCsdText(text.toLocal8Bit());
}

int QCsound::compileOrc(const QString &text) {
    return csound->CompileOrc(text.toLocal8Bit());
}

double QCsound::evalCode(const QString &text) {
    return csound->EvalCode(text.toLocal8Bit());
}

double QCsound::get0dBFS() {
    return csound->Get0dBFS(); //cs->Get0dBFS();
}

int QCsound::getApiVersion() {
    return csound->GetAPIVersion();
}

double QCsound::getControlChannel(const QString &name) {
    int result = 0;
    double value = csound->GetControlChannel(name.toLocal8Bit(), &result);
    return value;
}

CSOUND* QCsound::getCsound() {
    return csound->GetCsound();
}

qint64 QCsound::getCurrentTimeSamples() { // FIXME: unknown type int64_t qint64
    return csound->GetCurrentTimeSamples();
}

QString QCsound::getEnv(const QString &name) { // not sure, if it works... test with setGlobalEnv
    return csound->GetEnv(name.toLocal8Bit());
}

int QCsound::getKsmps() {
    return csound->GetKsmps();
}

int QCsound::getNchnls() {
    return csound->GetNchnls();
}

int QCsound::getNchnlsInput() {
    return csound->GetNchnlsInput();
}

QString QCsound::getOutputName() {
    return QString(csound->GetOutputName());
}

double QCsound::getScoreOffsetSeconds() {
    return csound->GetScoreOffsetSeconds();
}

double QCsound::getScoreTime() {
    return csound->GetScoreTime();
}

int QCsound::getSr() {
    return csound->GetSr();
}

QString QCsound::getStringChannel(const QString &name) {
    char buffer[0x100];
    csound->GetStringChannel(name.toLocal8Bit(), buffer);
    return QString(buffer);
}

int QCsound::getVersion() {
    return csound->GetVersion();
}

bool QCsound::isPlaying() {
    return csound->IsPlaying();
}

int QCsound::isScorePending() {
    return csound->IsScorePending();
}

void QCsound::message(const QString &text) {
    csound->Message("%s", text.toLocal8Bit().constData());
}

int QCsound::perform() {
    // Perform in a separate thread of execution.
    return csound->PerformAndReset();
}

int QCsound::performKsmps() {
    return csound->PerformKsmps();
}

int QCsound::readScore(const QString &text) {
    csound->ReadScore(text.toLocal8Bit());
    return 0;
}

void QCsound::reset() {
    csound->Reset();
}

void QCsound::rewindScore() {
    csound->RewindScore();
}

int QCsound::runUtility(const QString &command, int argc, char **argv) {
    return csound->RunUtility(command.toLocal8Bit(), argc, argv); // probably does not work from JS due char **
}

int QCsound::scoreEvent(char opcode, const double *pfields, long field_count) {
    csound->ScoreEvent(opcode, pfields, field_count);
    return 0;
}

void QCsound::setControlChannel(const QString &name, double value) {
    csound->SetControlChannel(name.toLocal8Bit(), value);
}

int QCsound::setGlobalEnv(const QString &name, const QString &value) {
    return csound->SetGlobalEnv(name.toLocal8Bit(), value.toLocal8Bit());
}

void QCsound::setHostData(void *host_data) {
    csound->SetHostData(host_data);
}

void QCsound::setInput(const QString &name){
    csound->SetInput(name.toLocal8Bit());
}

void QCsound::setMessageCallback(void (*messageCallback)(CSOUND *csound, int level, const char *format, va_list valist)){
    csound->SetMessageCallback(messageCallback);
}

void QCsound::setMessageCallback(QObject *object) {
    qDebug() << "setMessageCallback with " << object;
}

int QCsound::setOption(const QString &name){
    return csound->SetOption(name.toLocal8Bit().data());
}

void QCsound::setOutput(const QString &name, const QString &type, const QString &format){
    csound->SetOutput(name.toLocal8Bit(), type.toLocal8Bit(), format.toLocal8Bit());
}

void QCsound::setScoreOffsetSeconds(double value){
    csound->SetScoreOffsetSeconds(value);
}

void QCsound::setScorePending(bool value){
    csound->SetScorePending((int) value);
}

void QCsound::setStringChannel(const QString &name, const QString &value){
    csound->SetStringChannel(name.toLocal8Bit(), value.toLocal8Bit().data());
}

int QCsound::start(){
    int result = 0;
    result = csound->Start();
    return result;
}

void QCsound::stop(){
    csound->Stop();
    csound->Join();
}

double QCsound::tableGet(int table_number, int index){
    return csound->TableGet(table_number, index);
}

int QCsound::tableLength(int table_number){
    return csound->TableLength(table_number);
}

void QCsound::tableSet(int table_number, int index, double value){
    csound->TableSet(table_number, index, value);
}

void QCsound::csoundMessageCallback_(CSOUND *csound,
                                         int attributes,
                                         const char *format,
                                         va_list args) {
    return reinterpret_cast<QCsound *>(csoundGetHostData(csound))->csoundMessageCallback(attributes, format, args);
}

void QCsound::csoundMessageCallback(int attributes,
                           const char *format,
                           va_list args)
{
    (void) attributes;
    QString message = QString::vasprintf(format, args);
    qDebug() << message;
    passMessages(message);
    for (int i = 0, n = message.length(); i < n; i++) {
        if (message[i] == '\n') {
            QString code = QString("console.log(\"%1\\n\");").arg(csound_messages_buffer);
            csound_web_view->evaluateJavaScript(code);
            csound_messages_buffer.clear();
        } else {
            csound_messages_buffer.append(message[i]);
        }
    }
}

void QCsound::setMetadata(const QString &key, const QString &value)
{
    csound->SetMetadata(key.toStdString(), value.toStdString());
}

QString QCsound::getMetadata(const QString &key) const
{
    return csound->GetMetadata(key.toStdString()).c_str();
}

void QCsound::setDoGitCommit(bool do_git_commit) {
    csound->SetDoGitCommit(do_git_commit);
}

bool QCsound::getDoGitCommit() const {
    return csound->GetDoGitCommit();
}

int QCsound::Cleanup() {
    return csound->Cleanup();
}

int QCsound::CompileCsd(const QString &filename) {
    return csound->CompileCsd(filename.toLocal8Bit());
}

int QCsound::CompileCsdText(const QString &text) {
    return csound->CompileCsdText(text.toLocal8Bit());
}

int QCsound::CompileOrc(const QString &text) {
    return csound->CompileOrc(text.toLocal8Bit());
}

double QCsound::EvalCode(const QString &text) {
    return csound->EvalCode(text.toLocal8Bit());
}

double QCsound::Get0dBFS() {
    return csound->Get0dBFS(); //cs->Get0dBFS();
}

int QCsound::GetApiVersion() {
    return csound->GetAPIVersion();
}

double QCsound::GetControlChannel(const QString &name) {
    int result = 0;
    double value = csound->GetControlChannel(name.toLocal8Bit(), &result);
    return value;
}

CSOUND* QCsound::GetCsound() {
    return csound->GetCsound();
}

qint64 QCsound::GetCurrentTimeSamples() { // FIXME: unknown type int64_t qint64
    return csound->GetCurrentTimeSamples();
}

QString QCsound::GetEnv(const QString &name) { // not sure, if it works... test with setGlobalEnv
    return csound->GetEnv(name.toLocal8Bit());
}

int QCsound::GetKsmps() {
    return csound->GetKsmps();
}

int QCsound::GetNchnls() {
    return csound->GetNchnls();
}

int QCsound::GetNchnlsInput() {
    return csound->GetNchnlsInput();
}

QString QCsound::GetOutputName() {
    return QString(csound->GetOutputName());
}

double QCsound::GetScoreOffsetSeconds() {
    return csound->GetScoreOffsetSeconds();
}

double QCsound::GetScoreTime() {
    return csound->GetScoreTime();
}

int QCsound::GetSr() {
    return csound->GetSr();
}

QString QCsound::GetStringChannel(const QString &name) {
    char buffer[0x100];
    csound->GetStringChannel(name.toLocal8Bit(), buffer);
    return QString(buffer);
}

int QCsound::GetVersion() {
    return csound->GetVersion();
}

bool QCsound::IsPlaying() {
    return csound->IsPlaying();
}

int QCsound::IsScorePending() {
    return csound->IsScorePending();
}

void QCsound::Message(const QString &text) {
    csound->Message("%s", text.toLocal8Bit().constData());
}

int QCsound::Perform() {
    // Perform in a separate thread of execution.
    return csound->PerformAndReset();
}

int QCsound::PerformKsmps() {
    return csound->PerformKsmps();
}

int QCsound::ReadScore(const QString &text) {
    csound->ReadScore(text.toLocal8Bit());
    return 0;
}

void QCsound::Reset() {
    csound->Reset();
}

void QCsound::RewindScore() {
    csound->RewindScore();
}

int QCsound::RunUtility(const QString &command, int argc, char **argv) {
    return csound->RunUtility(command.toLocal8Bit(), argc, argv); // probably does not work from JS due char **
}

int QCsound::ScoreEvent(char opcode, const double *pfields, long field_count) {
    csound->ScoreEvent(opcode, pfields, field_count);
    return 0;
}

void QCsound::SetControlChannel(const QString &name, double value) {
    csound->SetControlChannel(name.toLocal8Bit(), value);
}

int QCsound::SetGlobalEnv(const QString &name, const QString &value) {
    return csound->SetGlobalEnv(name.toLocal8Bit(), value.toLocal8Bit());
}

void QCsound::SetHostData(void *host_data) {
    csound->SetHostData(host_data);
}

void QCsound::SetInput(const QString &name){
    csound->SetInput(name.toLocal8Bit());
}

void QCsound::SetMessageCallback(void (*messageCallback)(CSOUND *csound, int level, const char *format, va_list valist)){
    csound->SetMessageCallback(messageCallback);
}

void QCsound::SetMessageCallback(QObject *object) {
    qDebug() << "setMessageCallback with " << object;
}

int QCsound::SetOption(const QString &name){
    return csound->SetOption(name.toLocal8Bit().data());
}

void QCsound::SetOutput(const QString &name, const QString &type, const QString &format){
    csound->SetOutput(name.toLocal8Bit(), type.toLocal8Bit(), format.toLocal8Bit());
}

void QCsound::SetScoreOffsetSeconds(double value){
    csound->SetScoreOffsetSeconds(value);
}

void QCsound::SetScorePending(bool value){
    csound->SetScorePending((int) value);
}

void QCsound::SetStringChannel(const QString &name, const QString &value){
    csound->SetStringChannel(name.toLocal8Bit(), value.toLocal8Bit().data());
}

int QCsound::Start(){
    int result = 0;
    result = csound->Start();
    return result;
}

void QCsound::Stop(){
    csound->Stop();
    csound->Join();
}

double QCsound::TableGet(int table_number, int index){
    return csound->TableGet(table_number, index);
}

int QCsound::TableLength(int table_number){
    return csound->TableLength(table_number);
}

void QCsound::TableSet(int table_number, int index, double value){
    csound->TableSet(table_number, index, value);
}

void QCsound::SetMetadata(const QString &key, const QString &value)
{
    csound->SetMetadata(key.toStdString(), value.toStdString());
}

QString QCsound::GetMetadata(const QString &key) const
{
    return csound->GetMetadata(key.toStdString()).c_str();
}

void QCsound::SetDoGitCommit(bool do_git_commit) {
    csound->SetDoGitCommit(do_git_commit);
}

bool QCsound::GetDoGitCommit() const {
    return csound->GetDoGitCommit();
}

int QCsound::PerformAndPostProcess() {
    return csound->PerformAndPostProcess();
}



