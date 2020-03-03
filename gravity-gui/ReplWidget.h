#ifndef TERMINALWIDGET_H
#define TERMINALWIDGET_H
/*
Copyright 2005-2020 Kendall F. Morris

This file is part of the USF Gravity Gui software suite.

    The Gravity Gui suite is free software: you can redistribute
    it and/or modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation, either
    version 3 of the License, or (at your option) any later version.

    The suite is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with the suite.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <QtWidgets/QTextEdit>
#include <QKeyEvent>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocumentFragment>
#include <QStack>
#include <QString>

class ReplWidget : public QTextEdit
{
  Q_OBJECT

public:
  ReplWidget(QWidget *parent = 0);
  virtual ~ReplWidget();
  QString prompt() const;
  void setPrompt(const QString &prompt);
  void clearScreen();
  void reset();
  void printWarn(const QString& msg);
  void fakeEnter();

protected:
  void keyPressEvent(QKeyEvent *e);

  // Do not handle other events
  void mousePressEvent(QMouseEvent *)       { /* Ignore */ }
  void mouseDoubleClickEvent(QMouseEvent *) { /* Ignore */ }
  void mouseMoveEvent(QMouseEvent *)        { /* Ignore */ }
  void mouseReleaseEvent(QMouseEvent *)     { /* Ignore */ }

private:
  void handleLeft(QKeyEvent *event);
  void handleEnter();
  void handleHistoryUp();
  void handleHistoryDown();
  void handleHome();
  void moveToPromptStart();
  void moveToEndOfLine();
  void clearLine();
  QString getCommand();

  int getIndex (const QTextCursor &crQTextCursor );

  QString userPrompt;
  QStack<QString> historyUp;
  QStack<QString> historyDown;
  bool allowStdin, historySkip;
  int currentInsert=0;

// The command signal is fired when a user input is entered
signals:
  void command(QString command);

// The result slot displays the result of a command in the terminal
public slots:
  void result(QString result, bool showPrompt = true);
  void append(QString text);
  void defaultResponse(QString text);
};

#endif
