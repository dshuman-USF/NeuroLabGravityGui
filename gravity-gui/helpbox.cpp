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

#include <QDesktopServices>


#include "helpbox.h"
#include "ui_helpbox.h"

helpbox::helpbox(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::helpbox)
{
    ui->setupUi(this);
}

helpbox::~helpbox()
{
    delete ui;
}


void helpbox::on_helpClose_2_accepted()
{

}

void helpbox::on_helpText_anchorClicked(const QUrl &arg1)
{
    QDesktopServices::openUrl(QUrl(arg1));
}
