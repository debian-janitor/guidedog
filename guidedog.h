/***************************************************************************
                          guidedog.h  -  description
                             -------------------
    begin                : Thu Sep 27 22:23:16 CEST 2001
    copyright            : (C) 2001 by Simon Edwards
    email                : simon@simonzone.com
 ***************************************************************************/
/***************************************************************************
    Converted to a pure Qt5 application
    begin                : December 2014
    by                   : Antonio Martins
    email                : digiplan.pt@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef GUIDEDOG_H
#define GUIDEDOG_H

#include "simelineedit.h"
#include "guidedogdoc.h"

#include <QDialog>
#include <QListWidget>
#include <QCheckBox>
#include <QValidator>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define SYSTEM_RC_GUIDEDOG "/etc/rc.guidedog"
#define GUIDEDOG_VERSION "1.3.0"


/*!
 * \brief The AddressValidator class that validates an IP address
 */
class AddressValidator : public QValidator {
    Q_OBJECT
public:
    explicit AddressValidator(QWidget *parent = 0);
    ~AddressValidator();

    virtual State validate(QString &input, int & pos) const;
    virtual void fixup(QString &input) const;
};


/*!
 * \brief The IPValidator class
 */
class IPValidator : public QValidator {
    Q_OBJECT
public:
    explicit IPValidator(QWidget *parent = 0);
    ~IPValidator();

    virtual State validate(QString &input, int & pos) const;
    virtual void fixup(QString &input) const;
};


namespace Ui {
class GuideDogApp;
}


/*!
 * \brief The GuideDogApp class is the base class of the project
 */
class GuideDogApp : public QDialog {
    Q_OBJECT
public:
    explicit GuideDogApp(QWidget *parent = 0);
    ~GuideDogApp();

    bool initialize();
    void show();

protected:
    void saveAppOptions();
    void readOptions();

public slots:
    // Dialog main buttons
    void slotApplySave();       // Writes, executes and exits
    void slotApply();           // Execute only
    void slotClose();           // Close only
    void slotAbout();
    void slotHelp();

    // Routing
    void slotRouting(bool on);
    void slotMasquerade(bool on);
    void slotMasqueradeFTP(bool on);
    void slotMasqueradeIRC(bool on);

    void slotNoMasqueradeListBox(QListWidgetItem *item);
    void slotNewAddressButton();
    void slotDeleteAddressButton();
    void slotAddressLineEdit(const QString &s);
    void slotAddressLineEditReturn();

    // Forwarding
    void slotForwardListBox(QListWidgetItem *item);
    void slotNewForwardButton();
    void slotDeleteForwardButton();

    // Original destination
    void slotOriginalPortSpinBox(int x);
    void slotOriginalMachineRadio();
    void slotOriginalSpecifyRadio();
    void slotOriginalSpecifyLineEdit(const QString &);
    void slotPortProtocolComboBox(int x);

    // New destination
    void slotNewMachineRadio();
    void slotNewSpecifyRadio();
    void slotNewSpecifyLineEdit(const QString &s);
    void slotNewPortSpinBox(int x);

    // Comment
    void slotCommentLineEdit(const QString &);
                 
    // Import/Export
    void slotDisableGuidedog(bool on);
    void slotImportButton();
    void slotExportButton();
    void slotDescriptionChanged();

private:
    // bool superusermode;
    bool systemconfigmodified;
	bool waspreviousconfiguration;

    GuidedogDoc *doc;
    bool updatinggui;
    bool isSuperUser;

    void syncGUIFromDoc();

    void enabledGUIStuff();
    void setForwardRule(const GuidedogPortForwardRule *rule);
    void checkRootPrivileges();

    bool applyScript(bool warnfirst);
    bool saveScript();
    void openDefault();
    bool resetSystemConfiguration();

    Ui::GuideDogApp *ui;
    // The widgets that were previously here have been replaced by the ones in the ui
    AddressValidator *addressvalidator;
    IPValidator *originalspecifyaddressvalidator;
    IPValidator *newspecifyaddressvalidator;
    QSize commandrunnersize;
};

#endif // GUIDEDOG_H
