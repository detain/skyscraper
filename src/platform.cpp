/***************************************************************************
 *            platform.cpp
 *
 *  Sat Dec 23 10:00:00 CEST 2017
 *  Copyright 2017 Lars Muldjord
 *  muldjordlars@gmail.com
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

#include <iostream>

#include <QByteArray>
#include <QFile>
#include <QDir>
#include <QProcess>
#include <QString>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "platform.h"

void Platform::loadConfig(const QString& configPath)
{
    clearConfigData();

    QFile configFile(configPath);
    if (!configFile.open(QIODevice::ReadOnly))
        return;

    QByteArray saveData = configFile.readAll();
    QJsonDocument json(QJsonDocument::fromJson(saveData));

    if(json.isNull() || json.isEmpty())
        return;

    QJsonArray platformsArray = json["platforms"].toArray();
    for (int platformIndex = 0; platformIndex < platformsArray.size(); ++platformIndex) {
        QJsonObject platformObject = platformsArray[platformIndex].toObject();

        QString platformName = platformObject["name"].toString();
        platforms.push_back(platformName);

        QJsonArray scrapersArray = platformObject["scrapers"].toArray();
        for (int scraperIndex = 0; scraperIndex < scrapersArray.size(); ++scraperIndex) {
            QString scraperName = scrapersArray[scraperIndex].toString();
            platformToScrapers[platformName].push_back(scraperName);
}

        QJsonArray formatsArray = platformObject["formats"].toArray();
        for (int formatIndex = 0; formatIndex < formatsArray.size(); ++formatIndex) {
            QString formatName = formatsArray[formatIndex].toString();
            platformToFormats[platformName].push_back(formatName);
        }

        QJsonArray aliasesArray = platformObject["aliases"].toArray();
        for (int aliasIndex = 0; aliasIndex < aliasesArray.size(); ++aliasIndex) {
            QString aliasName = aliasesArray[aliasIndex].toString();
            platformToAliases[platformName].push_back(aliasName);
        }
    }
    platforms.sort();
}

void Platform::clearConfigData()
{
    platforms.clear();
    platformToScrapers.clear();
    platformToFormats.clear();
    platformToAliases.clear();
}

Platform& Platform::get()
{
    static Platform platform;
    return platform;
}

QStringList Platform::getPlatforms() const
{
  return platforms;
}

QStringList Platform::getScrapers(QString platform) const
{
    QStringList scrapers = platformToScrapers[platform];

  // Always add 'cache' as the last one
  scrapers.append("cache");

  return scrapers;
}

QString Platform::getFormats(QString platform, QString extensions, QString addExtensions) const
{
  if(!extensions.isEmpty() && extensions.contains("*.")) {
    return extensions;
  }

  QString formats = "*.zip *.7z *.ml "; // The last ' ' IS IMPORTANT!!!!!
  if(!addExtensions.isEmpty() && addExtensions.contains("*.")) {
    formats.append(addExtensions);
  }
  if(formats.right(1) != " ") {
    formats.append(" ");
  }
  QStringList myFormats = platformToFormats[platform];
  if(!myFormats.isEmpty())
  {
    int count = myFormats.size();
    for(int i = 0; i < count - 1; ++i)
      formats.append(myFormats[i] + " ");
    formats.append(myFormats[count - 1]);
  }
  return formats;
}

// If user provides no scraping source with '-s' this sets the default for the platform
QString Platform::getDefaultScraper() const
{
  return "cache";
}

// This contains all known platform aliases as listed on each of the scraping source sites
QStringList Platform::getAliases(QString platform) const
{
  QStringList aliases;
  // Platform name itself is always appended as the first alias
  aliases.append(platform);
  aliases.append(platformToAliases[platform]);
  return aliases;
}

// --- Console colors ---
// Black        0;30     Dark Gray     1;30
// Red          0;31     Light Red     1;31
// Green        0;32     Light Green   1;32
// Brown/Orange 0;33     Yellow        1;33
// Blue         0;34     Light Blue    1;34
// Purple       0;35     Light Purple  1;35
// Cyan         0;36     Light Cyan    1;36
// Light Gray   0;37     White         1;37
