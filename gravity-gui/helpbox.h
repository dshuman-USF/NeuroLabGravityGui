#ifndef HELPBOX_H
#define HELPBOX_H

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

#include <QDialog>

namespace Ui {
class helpbox;
}

class helpbox : public QDialog
{
    Q_OBJECT

public:
    explicit helpbox(QWidget *parent = 0);
    ~helpbox();

private slots:

    void on_helpClose_2_accepted();

    void on_helpText_anchorClicked(const QUrl &arg1);

private:
    Ui::helpbox *ui;
};

#endif // HELPBOX_H
