/***************************************************************************
 *            miyoo.cpp
 *
 *  Jan 18 2023
 *  Copyright 2023 Nic Jansma
 *  https://nicj.net
 ****************************************************************************/
/*
 *  This file is part of skyscraper.
 *
 *  skyscraper is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  skyscraper is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with skyscraper; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.
 */

#include "miyoo.h"
#include "xmlreader.h"
#include "strtools.h"
#include "platform.h"

#include <QDir>
#include <QRegularExpression>

Miyoo::Miyoo()
{
}

bool Miyoo::loadOldGameList(const QString &gameListFileString)
{
  // Load old game list entries so we can preserve metadata later when assembling xml
  XmlReader gameListReader;
  if(gameListReader.setFile(gameListFileString)) {
    oldEntries = gameListReader.getEntries(config->inputFolder);
    return true;
  }

  return false;
}

bool Miyoo::skipExisting(QList<GameEntry> &gameEntries, QSharedPointer<Queue> queue) 
{
  gameEntries = oldEntries;

  printf("Resolving missing entries...");
  int dots = 0;
  for(int a = 0; a < gameEntries.length(); ++a) {
    dots++;
    if(dots % 100 == 0) {
      printf(".");
      fflush(stdout);
    }
    QFileInfo current(gameEntries.at(a).path);
    for(int b = 0; b < queue->length(); ++b) {
      if(current.isFile()) {
	if(current.fileName() == queue->at(b).fileName()) {
	  queue->removeAt(b);
	  // We assume filename is unique, so break after getting first hit
	  break;
	}
      } else if(current.isDir()) {
	// Use current.absoluteFilePath here since it is already a path. Otherwise it will use
	// the parent folder
	if(current.absoluteFilePath() == queue->at(b).absolutePath()) {
	  queue->removeAt(b);
	  // We assume filename is unique, so break after getting first hit
	  break;
	}
      }
    }
  }
  return true;
}

void Miyoo::preserveFromOld(GameEntry &entry)
{
  // NOP
  if(entry.eSFavorite.isEmpty()) {
    return;
  }
}

void Miyoo::assembleList(QString &finalOutput, QList<GameEntry> &gameEntries)
{
  int dots = 0;
  // Always make dotMod at least 1 or it will give "floating point exception" when modulo
  int dotMod = gameEntries.length() * 0.1 + 1;
  if(dotMod == 0)
    dotMod = 1;
  finalOutput.append("<?xml version=\"1.0\"?>\n<gameList>\n");
  for(auto &entry: gameEntries) {
    if(dots % dotMod == 0) {
      printf(".");
      fflush(stdout);
    }
    dots++;

    QString entryType = "game";

    QFileInfo entryInfo(entry.path);
    if(entryInfo.isFile() && config->platform != "daphne") {
      // Check if game is in subfolder. If so, change entry to <folder> type.
      QString entryAbsolutePath = entryInfo.absolutePath();
      // Check if path is exactly one subfolder beneath root platform folder (has one more '/')
      if(entryAbsolutePath.count("/") == config->inputFolder.count("/") + 1) {
	QString extensions = Platform::get().getFormats(config->platform,
						  config->extensions,
						  config->addExtensions);
	// Check if the platform has both cue and bin extensions. Remove bin if it does to avoid count() below to be 2
	// I thought about removing bin extensions entirely from platform.cpp, but I assume I've added them per user request at some point.
	if(extensions.contains("*.cue") &&
	   extensions.contains("*.bin")) {
	  extensions.replace("*.bin", "");
	  extensions = extensions.simplified();
	}
	// Check is subfolder has more roms than one, in which case we stick with <game>
	if(QDir(entryAbsolutePath, extensions).count() == 1) {
	  entryType = "folder";
	  entry.path = entryAbsolutePath;
	}
      }
    } else if(entryInfo.isDir()) {
      entryType = "folder";
    }

    // Preserve certain data from old game list entry, but only for empty data
    preserveFromOld(entry);

    if(config->platform == "daphne") {
      entry.path.replace("daphne/roms/", "daphne/").replace(".zip", ".daphne");
      entryType = "game";
    }

    if(config->relativePaths) {
      entry.path.replace(config->inputFolder, ".");
    }

    finalOutput.append("  <" + entryType + ">\n");
    finalOutput.append("    <path>" + StrTools::xmlEscape(entry.path) + "</path>\n");
    finalOutput.append("    <name>" + StrTools::xmlEscape(entry.title) + "</name>\n");
    if(entry.screenshotFile.isEmpty()) {
      finalOutput.append("    <image />\n");
    } else {
      finalOutput.append("    <image>" + (config->relativePaths?StrTools::xmlEscape(entry.screenshotFile).replace(config->inputFolder, "."):StrTools::xmlEscape(entry.screenshotFile)) + "</image>\n");
    }
    finalOutput.append("  </" + entryType + ">\n");
  }
  finalOutput.append("</gameList>");
}

bool Miyoo::canSkip()
{
  return true;
}

QString Miyoo::getGameListFileName()
{
  return QString("miyoogamelist.xml");
}

QString Miyoo::getInputFolder()
{
  return QString(QDir::homePath() + "/RetroPie/roms/" + config->platform);
}

QString Miyoo::getGameListFolder()
{
  return config->inputFolder;
}

QString Miyoo::getCoversFolder()
{
  return NULL;
}

QString Miyoo::getScreenshotsFolder()
{
  return this->getGameListFolder() + "/Imgs";
}

QString Miyoo::getWheelsFolder()
{
  return NULL;
}

QString Miyoo::getMarqueesFolder()
{
  return NULL;
}

QString Miyoo::getTexturesFolder() {
  return NULL;
}

QString Miyoo::getVideosFolder()
{
  return NULL;
}
