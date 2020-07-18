/******************************************************************************
    Copyright (C) 2014 by Hugh Bailey <obs.jim@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/

#include <QDesktopServices>
#include <QLabel>
#include "window-permissions.hpp"
#include "obs-app.hpp"

OBSPermissions::OBSPermissions(QWidget *parent)
	: QDialog(parent), ui(new Ui::OBSPermissions)
{
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
	ui->setupUi(this);
}

void OBSPermissions::SetStatus(QLabel *label, QPushButton *btn, bool allowed)
{
	if (allowed) {
		label->setText("Allowed");
	} else {
		label->setText("Not Allowed");
	}
	btn->setEnabled(!allowed);
}

void OBSPermissions::setPermissions(bool video, bool audio, bool capture, bool input)
{
	SetStatus(ui->capturePermissionStatus, ui->capturePermissionButton, capture);
	SetStatus(ui->videoPermissionStatus, ui->videoPermissionButton, video);
	SetStatus(ui->audioPermissionStatus, ui->audioPermissionButton, audio);
	SetStatus(ui->inputPermissionStatus, ui->inputPermissionButton, input);

}

void OBSPermissions::on_closeButton_clicked()
{
	close();
}