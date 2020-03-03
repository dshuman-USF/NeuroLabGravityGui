/* Copyright (c) 2011 Peter Braun

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

// substantially modifed by dshuman Aug 2017 to make it
// more of a [Print prompt] then [Wait for input] widget.

#include <iostream>
#include <QColor>
#include <QFont>
//#include <QApplication>
#include "ReplWidget.h"

using namespace std;

ReplWidget::ReplWidget(QWidget *parent) : QTextEdit(parent),
  userPrompt(QString("> ")),
  allowStdin(false),
  historySkip(false)
{
  historyUp.clear();
  historyDown.clear();
  setLineWrapMode(NoWrap);
  insertPlainText(userPrompt);
}

ReplWidget::~ReplWidget()
{

}

/** Filter all key events from stdin. The keys are filtered and handled manually
  * in order to create a typical shell-like behaviour. For example
  * Up and Down arrows don't move the cursor, but allow the user to
  * browse the last commands that there launched.
  * Note that in our usage here, our gravity_gui wrapper controls stdin and
  * can send user input, text from files, command line args, whatever.
  */
void ReplWidget::keyPressEvent(QKeyEvent *e) {

     // handle control-C unconditionally
  if ( (e->modifiers() & Qt::ControlModifier) && e->key() == Qt::Key_C) {
     append("^C\n"+userPrompt);
     QString cmd (0x03);  // ascii ETX, aka, ^C
     emit command(cmd);
     return;
  }

  // allowStdin state: prompt is displayed, no response has been received yet.
  if(!allowStdin)
     return;

  switch(e->key()) {
  case Qt::Key_Return:   // kbd
  case Qt::Key_Enter:    // kpad
    handleEnter();
    break;
  case Qt::Key_Backspace:
    handleLeft(e);
    break;
  case Qt::Key_Up:
    handleHistoryUp();
    break;
  case Qt::Key_Down:
    handleHistoryDown();
    break;
  case Qt::Key_Left:
    handleLeft(e);
    break;
  case Qt::Key_Home:
    handleHome();
    break;
  default:
    QTextEdit::keyPressEvent(e);
    break;
  }
}

// Enter key pressed. Chars that have come in from stdin (if any) now needs
// to be processed by the terminal widget's owner.
void ReplWidget::handleEnter() {
  QString cmd = getCommand();

  if(0 < cmd.length()) {
    while(historyDown.count() > 0) {
      historyUp.push(historyDown.pop());
    }
    historyUp.push(cmd);
  }

  moveToEndOfLine();
  allowStdin = false;    // this means we stop accepting input until
  setFocus();            // new prompt text shows up
  insertPlainText("\n");
  emit command(cmd);
}

// Print a prompt for the user. Optionally do not display prompt char,
// is displayed by default.
void ReplWidget::result(QString result,bool showprompt) {
  insertPlainText(result);
  if (showprompt)
     insertPlainText(userPrompt);
  ensureCursorVisible();
    // remember the end of the prompt.
    // User cannot backspace before this, and
    // this is where the response to the prompt starts from
  currentInsert = textCursor().anchor();
  allowStdin = true;  // stdin input now allowed
}

// Use this to display information that does not require user input
// This does not control stdin input allowed/not allowed.
void ReplWidget::append(QString text) {
  insertPlainText(text);
  currentInsert = this->textCursor().anchor();
  ensureCursorVisible();
//  QApplication::processEvents(); // how to flush output
}

// Use this to add a default response.
// Assumes result has been called before this.
void ReplWidget::defaultResponse(QString text) {
  insertPlainText(text);
  ensureCursorVisible();
}

void ReplWidget::printWarn(const QString& msg)
{
   QColor color = this->textColor();
   int wght = fontWeight();
   setTextColor(Qt::blue);
   setFontWeight(QFont::Bold);
   append(msg);
   setTextColor(color);
   setFontWeight(wght);
//   QApplication::processEvents(); // how to flush output
}

// Provide for case where we want to press Enter for user
void ReplWidget::fakeEnter() {
   handleEnter();
}


void ReplWidget::clearScreen() {
   clear();
   insertPlainText(userPrompt);
   ensureCursorVisible();
   currentInsert = this->textCursor().anchor();
}

// Arrow up pressed
void ReplWidget::handleHistoryUp() {

  if(0 < historyUp.count()) {
     moveToPromptStart();
     QString cmd = historyUp.pop();
     historyDown.push(cmd);
     insertPlainText(cmd);
   }
   historySkip = true;
}

// Arrow down pressed
void ReplWidget::handleHistoryDown() {
  if(0 < historyDown.count() && historySkip) {
    historyUp.push(historyDown.pop());
    historySkip = false;
  }

  if(0 < historyDown.count()) {
    QString cmd = historyDown.pop();
    historyUp.push(cmd);
    moveToPromptStart();
    insertPlainText(cmd);
  }
  else {
    moveToPromptStart();
    insertPlainText("");
  }
}

void ReplWidget::moveToPromptStart() {
   QTextCursor c = textCursor();
   int beg, end;
   beg = currentInsert;
   end = c.anchor();
   c.setPosition(beg, QTextCursor::MoveAnchor);
   c.setPosition(end, QTextCursor::KeepAnchor);
   setTextCursor(c);
}


void ReplWidget::clearLine() {
  QTextCursor c = this->textCursor();
  c.select(QTextCursor::LineUnderCursor);
  c.removeSelectedText();
  this->insertPlainText(userPrompt);
}


// Select and return the user-input
QString ReplWidget::getCommand() {
  moveToPromptStart();
  QTextCursor c = textCursor();
  QString text = c.selectedText();
  return text;
}

void ReplWidget::moveToEndOfLine() {
  QTextEdit::moveCursor(QTextCursor::EndOfLine);
}

// The text cursor is not allowed to move before the prompt
void ReplWidget::handleLeft(QKeyEvent *event) {
  auto now = textCursor().anchor();
  if (now > currentInsert) {
    QTextEdit::keyPressEvent(event);
  }
}

// Home (pos1) key pressed
void ReplWidget::handleHome() {
  QTextCursor c = textCursor();
  c.movePosition(QTextCursor::StartOfLine);
  c.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, currentInsert);
//  c.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, userPrompt.length());
  setTextCursor(c);
}

// Solution for getting x and y position of the cursor. Found
// them in the Qt mailing list
int ReplWidget::getIndex (const QTextCursor &crQTextCursor ) {
  QTextBlock b;
  int column = 1;
  b = crQTextCursor.block();
  column = crQTextCursor.position() - b.position();
  return column;
}

void ReplWidget::setPrompt(const QString &prompt) {
  userPrompt = prompt;
  clearLine();
}

QString ReplWidget::prompt() const {
  return userPrompt;
}

// back to base state
void ReplWidget::reset() {
  allowStdin = false;
  historySkip = false;
  insertPlainText(userPrompt);
  currentInsert = textCursor().anchor();
}

