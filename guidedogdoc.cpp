/***************************************************************************
                          guidedogdoc.cpp  -  description
                             -------------------
    begin                : Thu Sep 27 2001
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

#include "guidedogdoc.h"
#include "guidedog.h"

#include <QFile>
#include <QSaveFile>

///////////////////////////////////////////////////////////////////////////
GuidedogPortForwardRule::GuidedogPortForwardRule() {
    specifyoriginal = false;
    specifynewaddress = false;
    iptype = IPPROTO_TCP;
    originalport = 0;
    newport = 0;
}
    
///////////////////////////////////////////////////////////////////////////
QString GuidedogPortForwardRule::getSummary() const {
    QString str;

    if (specifyoriginal) {
        str += originaladdress.getAddress();
        str += ":";
    }

    if (iptype == IPPROTO_TCP) {
        str += "TCP ";
    } else {
        str += "UDP ";    
    }
    
    str += QString::number(originalport);
    
    str += " -> ";

    if (specifynewaddress) {
        str += newaddress.getAddress();
        str += ":";
    }
    
    str += QString::number(newport);
    str += " ";
    str += comment;
    return str;
}

///////////////////////////////////////////////////////////////////////////
GuidedogDoc::GuidedogDoc() {
    factoryDefaults();
}

///////////////////////////////////////////////////////////////////////////
GuidedogDoc::~GuidedogDoc(){
}

///////////////////////////////////////////////////////////////////////////
void GuidedogDoc::setDisabled(bool on) {
    disabled = on;
}

///////////////////////////////////////////////////////////////////////////
bool GuidedogDoc::isDisabled() {
    return disabled;
}

///////////////////////////////////////////////////////////////////////////
void GuidedogDoc::setRouting(bool on) {
    routing = on;
}

///////////////////////////////////////////////////////////////////////////
bool GuidedogDoc::isRouting() {
    return routing;
}

///////////////////////////////////////////////////////////////////////////
void GuidedogDoc::setMasquerade(bool on) {
    masquerade = on;
}

///////////////////////////////////////////////////////////////////////////
bool GuidedogDoc::isMasquerade() {
    return masquerade;
}
///////////////////////////////////////////////////////////////////////////
void GuidedogDoc::setMasqueradeFTP(bool on) {
    masqueradeftp = on;
}

///////////////////////////////////////////////////////////////////////////
bool GuidedogDoc::isMasqueradeFTP() {
    return masqueradeftp;
}
///////////////////////////////////////////////////////////////////////////
void GuidedogDoc::setMasqueradeIRC(bool on) {
    masqueradeirc = on;
}

///////////////////////////////////////////////////////////////////////////
bool GuidedogDoc::isMasqueradeIRC() {
    return masqueradeirc;
}

///////////////////////////////////////////////////////////////////////////
void GuidedogDoc::factoryDefaults() {
    disabled = false;
    routing = false;
    masquerade = false;
    masqueradeftp = false;
    masqueradeirc = false;
    description = "";
}

///////////////////////////////////////////////////////////////////////////
bool GuidedogDoc::writeScript(QTextStream &stream) {
    int c, oldc, i;
    GuidedogPortForwardRule *rule;

    //stream.setEncoding(QTextStream::Latin1);
    stream << "#!/bin/sh\n"
        "# [Guidedog]\n"
		"# DO NOT EDIT!\n"
        "# This script was generated by \"Guidedog\" by Simon Edwards and Antonio Cardoso Martins\n"
        "# http://www.simonzone.com/software/guidedog/ This script requires Linux\n"
        "# kernel above or equal than 2.4.x and iptables.\n"
	    "#\n"
	    "# [Description]\n";
    c = 0;
    oldc = 0;
    while ((c = description.indexOf('\n', c)) >= 0) {
        stream << "#  " << description.mid(oldc, c - oldc) << "\n";
        oldc = c + 1;
        c++;
    }
    c = (int)description.length();
    stream << "#  " << description.mid(oldc, c - oldc) << "\n";
	
    stream << "# [Config]\n"
        "# DISABLED="<< (disabled ? 1 : 0) << "\n"
        "# ROUTING="<< (routing ? 1 : 0) << "\n"
        "# MASQUERADE=" << (masquerade ? 1 : 0) << "\n"
        "# MASQUERADEFTP=" << (masqueradeftp ? 1 : 0) << "\n"
        "# MASQUERADEIRC=" << (masqueradeirc ? 1 : 0) << "\n";

    for (i = 0; i < nomasqueradelist.count(); i++) {
        stream << "# NOMASQUERADEADDRESS=" << nomasqueradelist.at(i).getAddress() << "\n";
    }

    for (i = 0; i < forwardrulelist.count(); i++) {
        rule = &forwardrulelist[i];
        stream << "# [Forwarding]\n"
            "# TYPE=" << (rule->iptype == IPPROTO_TCP ? "TCP" : "UDP") << "\n"
            "# ORIGINALPORT=" << (rule->originalport) << "\n"
            "# SPECIFYORIGINALADDRESS=" << (rule->specifyoriginal ? "1" : "0") << "\n"
            "# ORIGINALADDRESS=" << (rule->originaladdress.getAddress()) << "\n"
            "# NEWPORT=" << (rule->newport) << "\n"
            "# SPECIFYNEWADDRESS=" << (rule->specifynewaddress) << "\n"
            "# NEWADDRESS=" << (rule->newaddress.getAddress()) << "\n"
            "# COMMENT=" << (rule->comment) << "\n";
    }
                        
    stream << "# [End]\n"
        "\n"
        "# Real code starts here\n"
        "# If you change the line below then also change the # DISABLED line above.\n";
    if (disabled) {
        stream << "DISABLE_GUIDEDOG=1\n";
    } else {
        stream << "DISABLE_GUIDEDOG=0\n";
    }

    stream << "if [ $DISABLE_GUIDEDOG -eq 0 ]; then\n"
        "# Set the path\n"
        "PATH=/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/sbin\n"
        "if test -z $GUIDEDOG_VERBOSE; then\n"
        "  GUIDEDOG_VERBOSE=0\n"
        "fi;\n"
        "logger -p auth.info -t guidedog Configuring routing now.\n"
        "[ $GUIDEDOG_VERBOSE -eq 1 ] && echo \"Resetting routing rules.\"\n"

            // Clear the nat table.
        "# Shut down routing\n"
        "echo 0 > /proc/sys/net/ipv4/ip_forward 2> /dev/null\n"
        "# iptables -t nat -P PREROUTING DROP\n"
        "# iptables -t nat -P POSTROUTING DROP\n"
        "# iptables -t nat -P OUTPUT DROP\n"
        "iptables -t mangle -P PREROUTING DROP\n"
        "iptables -t mangle -P POSTROUTING DROP\n"
        "# Delete any existing chains\n"
        "iptables -t nat -F\n"
        "iptables -t nat -X\n"
        "iptables -t mangle -F\n"
        "iptables -t mangle -X\n"
        "\n"
        "# Enable/disable routing\n";
    if (routing) {
        stream << "[ $GUIDEDOG_VERBOSE -eq 1 ] && echo \"" << QObject::tr("Enabling routing.") << "\"\n";
    } else {
        stream << "[ $GUIDEDOG_VERBOSE -eq 1 ] && echo \"" << QObject::tr("Disabling routing.") << "\"\n";
    }
        // Code to enable/disable routing.
    stream << "echo " << (routing ? 1 : 0) << " > /proc/sys/net/ipv4/ip_forward 2> /dev/null\n"
        "\n"
        "[ $GUIDEDOG_VERBOSE -eq 1 ] && echo \"Loading kernel modules.\"\n"
      	"modprobe iptable_nat\n";

    if (routing && masquerade) {
        if (masqueradeftp) {
            stream << "modprobe ip_nat_ftp\n";
        }
        if (masqueradeirc) {
            stream << "modprobe ip_nat_irc\n";
        }
    }
    stream << "# Port forwarding/Tunnelling\n"
        "iptables -t nat -N gforward\n"
        "iptables -t nat -N ftolocal\n"
        "iptables -t mangle -N gforward\n"
        "iptables -t mangle -N ftolocal\n"
        "\n"
        "# Switch the current language for a moment\n"
        "GUIDEDOG_BACKUP_LANG=$LANG\n"
        "GUIDEDOG_BACKUP_LC_ALL=$LC_ALL\n"
        "LANG=US\n"
        "LC_ALL=US\n"
        "export LANG\n"
        "export LC_ALL\n"

        "# Work out our local IPs.\n"
        "LOCAL_IP=\"`ifconfig | awk '/inet addr:/ { match(\\$0,/inet addr:[[:digit:]\\\\.]+/)\n"
        "printf \\\"%s\\\\n\\\", substr(\\$0,RSTART+10,RLENGTH-10) }'`\"\n"

        "# Restore the language setting\n"
        "LANG=$GUIDEDOG_BACKUP_LANG\n"
        "LC_ALL=$GUIDEDOG_BACKUP_LC_ALL\n"
        "export LANG\n"
        "export LC_ALL\n"
        "for X in $LOCAL_IP ; do\n"
        "    iptables -t nat -A gforward -d $X -j ftolocal\n"
        "    iptables -t mangle -A gforward -d $X -j ftolocal\n"
        "done\n";

    for (i = 0; i < forwardrulelist.count(); i++) {
        rule = &forwardrulelist[i];
        if (rule->specifyoriginal) {
            stream << "# " << (rule->comment) << "\n";
            if (rule->specifynewaddress) {
                stream << "iptables -t mangle -A gforward "
                "-p " << (rule->iptype == IPPROTO_TCP ? "tcp" : "udp") << " "
                "--dport " << (rule->originalport) << " "
                "-d " << (rule->originaladdress.getAddress()) << " "
                "-j MARK --set-mark 1\n";
            }
            stream << "iptables -t nat -A gforward "
                "-p " << (rule->iptype == IPPROTO_TCP ? "tcp" : "udp") << " "
                "--dport " << (rule->originalport) << " "
                "-d " << (rule->originaladdress.getAddress()) << " ";
            if (rule->specifynewaddress) {
                stream << "-j DNAT --to-destination " << (rule->newaddress.getAddress()) << ":";
            } else {
                // Redirecting to the local machine.
                stream << "-j REDIRECT --to-ports ";
            }
            stream << (rule->newport) << "\n";
        }
    }

    for (i = 0; i < forwardrulelist.count(); i++) {
        rule = &forwardrulelist[i];
        if (rule->specifyoriginal == false) {
            stream << "# " << (rule->comment) << "\n";
            if (rule->specifynewaddress) {
                stream << "iptables -t mangle -A ftolocal "
                    "-p " << (rule->iptype == IPPROTO_TCP ? "tcp" : "udp") << " "
                    "--dport " << (rule->originalport) << " "
                    "-j MARK --set-mark 1\n";
            }

            stream << "iptables -t nat -A ftolocal "
                "-p " << (rule->iptype == IPPROTO_TCP ? "tcp" : "udp") << " "
                "--dport " << (rule->originalport) << " ";
            if (rule->specifynewaddress) {
                stream << "-j DNAT --to-destination " << (rule->newaddress.getAddress()) << ":";
            } else {
                // Redirecting to the local machine.
                stream << "-j REDIRECT --to-ports ";
            }
            stream << (rule->newport) << "\n";
        }
    }
    stream << "iptables -t nat -A PREROUTING -j gforward\n"
        "iptables -t nat -A OUTPUT -j gforward\n"
        "iptables -t mangle -A PREROUTING -j gforward\n"
        "# Marked packets are in the process of being port forwarded.\n"
        "# We also need to source Nat them too.\n"
        "iptables -t nat -A POSTROUTING -m mark --mark 1 -j MASQUERADE\n";

        // Code to enable/disable masquerade.
    if (routing && masquerade) {
            // Fill up the NAT table with rules.
        stream << "[ $GUIDEDOG_VERBOSE -eq 1 ] && echo \"Inserting masquerade rules.\"\n"
	        "iptables -t nat -N fromprivate\n"
            "# Packets from the private IP range to another private IP range are untouched.\n"
            "iptables -t nat -A fromprivate -d 192.168.0.0/16 -j ACCEPT\n"
            "iptables -t nat -A fromprivate -d 172.16.0.0/12 -j ACCEPT\n"
            "iptables -t nat -A fromprivate -d 10.0.0.0/8 -j ACCEPT\n"
            "# Packets that get here are from the private address range\n"
            "# and are trying to get out to the internet. We NAT them.\n"
            "iptables -t nat -A fromprivate -j MASQUERADE\n"
            "\n";

        stream << "# No-masquerade addresses.\n";
        for (i = 0; i < nomasqueradelist.count(); i++) {
            stream << "iptables -t nat -A POSTROUTING -d " << nomasqueradelist[i].getAddress() << " -j ACCEPT\n";
        }

        stream << "# Siphon off any packets that are from the private IP range.\n"
            "iptables -t nat -A POSTROUTING -s 192.168.0.0/16 -j fromprivate\n"
            "iptables -t nat -A POSTROUTING -s 172.16.0.0/12 -j fromprivate\n"
            "iptables -t nat -A POSTROUTING -s 10.0.0.0/8 -j fromprivate\n"
            "# packets that get here can just hit the default policy.\n"
                "\n";
    }

    stream << "iptables -t nat -P PREROUTING ACCEPT\n"
        "iptables -t nat -P POSTROUTING ACCEPT\n"
        "iptables -t nat -P OUTPUT ACCEPT\n"
        "iptables -t mangle -P PREROUTING ACCEPT\n"
        "iptables -t mangle -P POSTROUTING ACCEPT\n"
        "iptables -t mangle -P OUTPUT ACCEPT\n"
        "[ $GUIDEDOG_VERBOSE -eq 1 ] && echo \"" << QObject::tr("Finished.") << "\"\n"
        "fi;\n"	// Matches the disable guidedog IF.
        "true\n";
    return true;
}


///////////////////////////////////////////////////////////////////////////
bool GuidedogDoc::readScript(QTextStream &stream, QString &errorstring) {
    QString s;
    bool addcr;
    bool ok;
    uint i;
    GuidedogPortForwardRule *rule;
    IPRange *nomasqaddress;
    const char *parameterlist[] = {
        "# DISABLED=",
        "# ROUTING=",
        "# MASQUERADE=",
        "# MASQUERADEFTP=",
        "# MASQUERADEIRC=",
        "# NOMASQUERADEADDRESS=",
        0};
    const char *forwardparameterlist[] = {
        "# TYPE=",
        "# ORIGINALPORT=",
        "# SPECIFYORIGINALADDRESS=",
        "# ORIGINALADDRESS=",
        "# NEWPORT=",
        "# SPECIFYNEWADDRESS=",
        "# NEWADDRESS=",
        "# COMMENT=",
        0
    };
    QString rightpart;

    rule=0;

    //stream.setEncoding(QTextStream::Latin1);
    s = stream.readLine();
    if (s.isNull()) {
        errorstring = QObject::tr("Error reading first line of the script.");
        goto error;
    }

    s = stream.readLine();
    if (s.isNull()) {
        errorstring = QObject::tr("Error reading second line of the script. Expected [Guidedog]");
        goto error;
    }

    if (s != "# [Guidedog]") {
        errorstring = QObject::tr("Error reading script. This does not appear to be a Guidedog script.");
        goto error;
    }

        // Read past the boring human readable copperplate stuff.
    do {
        s = stream.readLine();
        if (s.isNull()) {
            errorstring = QObject::tr("Error reading script. (Before Description section).");
            goto error;
        }
    } while (s != "# [Description]");

        // Read the config description.
    description = "";
    addcr = false;
    while (true) {
        s = stream.readLine();
        if (s.isNull()) {
            errorstring = QObject::tr("Error reading script. (In Description section).");
            goto error;
        }
        if (s == "# [Config]") {
            break;
        }
        if (addcr == true) {
            description.append("\n");
        }
        addcr = true;
        description.append(s.right(s.length() - 3));
    }

        // Read the Config
    s = stream.readLine();
    while (true) {
        if (s.isNull()) {
            errorstring = QObject::tr("Error reading script. (In Config section).");
            goto error;
        }
        if (s.startsWith("# [End]") || s.startsWith("# [Forwarding]")) {
            break;  // We've got to the end of this part of the show.
        }
            // Try to identify the line we are looking at.
        for (i = 0; parameterlist[i]!=0; i++) {
            if (s.startsWith(parameterlist[i])) {
                break;
            }
        }
        if (parameterlist[i] != 0) {
            rightpart = s.right(s.length() - strlen(parameterlist[i]));
        }
        switch(i) {
            case 0:
                // DISABLED
                disabled = rightpart == "1";
                break;
            case 1:
                // ROUTING
                routing = rightpart == "1";
                break;
            case 2:
                // MASQUERADE
                masquerade = rightpart == "1";
                break;
            case 3:
                // MASQUERADEFTP
                masqueradeftp = rightpart == "1";
                break;
            case 4:
                // MASQUERADEIRC
                masqueradeirc = rightpart == "1";
                break;
            case 5:
                // NOMASQUERADEADDRESS
                nomasqueradelist << IPRange();
                nomasqaddress = &nomasqueradelist.last();
                nomasqaddress->setAddress(rightpart);
                break;
            default:
                    // Should we complain?
                break;
        }
        s = stream.readLine();
    }

    while (true) {
        if (s.startsWith("# [Forwarding]")) {
            // Create a new forwarding rule.
            forwardrulelist << GuidedogPortForwardRule();
            rule = &forwardrulelist.last();
        }
        if (s.startsWith("# [End]")) {
            break;  // We've got to the end of this part of the show.
        }

            // Try to identify the line we are looking at.
        for (i = 0; forwardparameterlist[i] != 0; i++) {
            if (s.startsWith(forwardparameterlist[i])) {
                break;
            }
        }
        if (forwardparameterlist[i] != 0) {
            rightpart = s.right(s.length() - strlen(forwardparameterlist[i]));
        }
        switch(i) {
            case 0:
                // TYPE
                if (rightpart == "TCP") {
                    rule->iptype = IPPROTO_TCP;
                } else {
                    rule->iptype = IPPROTO_UDP;
                }
                break;
                
            case 1:
                // ORIGINALPORT
                rule->originalport = rightpart.toUInt(&ok);
                if (!ok) {
                    qDebug("Error in toUint conversion for originalport");
                }
                break;
                
            case 2:
                // SPECIFYORIGINALADDRESS
                rule->specifyoriginal = rightpart == "0" ? false : true;
                break;

            case 3:
                // ORIGINALADDRESS
                rule->originaladdress.setAddress(rightpart);
                break;
                
            case 4:
                // NEWPORT
                rule->newport = rightpart.toUInt(&ok);
                if (!ok) {
                    qDebug("Error in toUint conversion for newport");
                }
                break;
                
            case 5:
                // SPECIFYNEWADDRESS
                rule->specifynewaddress = rightpart == "0" ? false : true;
                break;
                
            case 6:
                // NEWADDRESS
                rule->newaddress.setAddress(rightpart);
                break;
                
            case 7:
                // COMMENT
                rule->comment = rightpart;
                break;
                
            default:
                break;
        }
        s = stream.readLine();
    }
    return true;

error:
    return false;
}

///////////////////////////////////////////////////////////////////////////
bool GuidedogDoc::openScript(const QString &filename, QString &errorstring) {
    QFile file(filename);

    if (file.open(QIODevice::ReadOnly)) {
        QTextStream stream(&file);

        if (readScript(stream, errorstring)) {
            file.close();
            return true;
        } else {
            file.close();
            return false;
        }
    } else {
        errorstring = QObject::tr("Unable to open the script from reading.");
        return false;
    }
}

///////////////////////////////////////////////////////////////////////////
bool GuidedogDoc::saveScript(const QString &filename, QString &errorstring) {
    QFile file(filename);     // We want it root executable.

    file.setPermissions(filename, QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner);
    if (!file.open(QIODevice::ReadWrite | QIODevice::Text)) {
        errorstring = QObject::tr("An error occurred while writing '%1'. The operating system has this to report about the error: %2")
        .arg(filename).arg(strerror(file.error()));
        return false;
    }

    QTextStream out(&file);
    if (writeScript(out)) {
        if (file.error() != 0) {
            errorstring = QObject::tr("An error occurred while writing '%1'. The operating system has this to report about the error: %2")
            .arg(filename).arg(strerror(file.error()));
		    return false;
        }
        file.close();
        if (file.error() != 0) {
            errorstring = QObject::tr("An error occurred while writing '%1'. The operating system has this to report about the error: %2")
            .arg(filename).arg(strerror(file.error()));
		    return false;
        }
        return true;
    } else {
        file.close();
        return false;
    }
}
