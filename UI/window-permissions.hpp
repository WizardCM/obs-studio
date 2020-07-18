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

#pragma once

#include "ui_OBSPermissions.h"
#include "platform.hpp"

class OBSPermissions : public QDialog {
	Q_OBJECT

private:
	std::unique_ptr<Ui::OBSPermissions> ui;
	void SetStatus(QLabel *label, QPushButton *btn, bool allowed);

public:
	OBSPermissions(QWidget *parent);
    void setPermissions(bool video, bool audio, bool capture, bool input);

	private slots:
	void on_closeButton_clicked();
	// void on_analyzeURL_clicked();
};
