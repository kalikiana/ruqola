/*
   Copyright (c) 2018-2019 Montel Laurent <montel@kde.org>

   This library is free software; you can redistribute it and/or modify
   it under the terms of the GNU Library General Public License as published
   by the Free Software Foundation; either version 2 of the License or
   ( at your option ) version 3 or, at the discretion of KDE e.V.
   ( which shall act as a proxy as in section 14 of the GPLv3 ), any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "setchanneltypejob.h"

#include "rocketchatqtrestapi_debug.h"
#include "restapimethod.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
using namespace RocketChatRestApi;
SetChannelTypeJob::SetChannelTypeJob(QObject *parent)
    : RestApiAbstractJob(parent)
{
}

SetChannelTypeJob::~SetChannelTypeJob()
{
}

bool SetChannelTypeJob::start()
{
    if (!canStart()) {
        deleteLater();
        return false;
    }
    const QByteArray baPostData = json().toJson(QJsonDocument::Compact);
    addLoggerInfo("SetChannelTypeJob::start: " + baPostData);
    QNetworkReply *reply = mNetworkAccessManager->post(request(), baPostData);
    connect(reply, &QNetworkReply::finished, this, &SetChannelTypeJob::slotSetGroupTypeFinished);
    return true;
}

void SetChannelTypeJob::slotSetGroupTypeFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if (reply) {
        const QByteArray data = reply->readAll();
        const QJsonDocument replyJson = QJsonDocument::fromJson(data);
        const QJsonObject replyObject = replyJson.object();

        if (replyObject[QStringLiteral("success")].toBool()) {
            addLoggerInfo(QByteArrayLiteral("SetChannelTypeJob success: ") + replyJson.toJson(QJsonDocument::Indented));
            Q_EMIT setGroupTypeDone();
        } else {
            emitFailedMessage(replyObject);
            addLoggerWarning(QByteArrayLiteral("SetChannelTypeJob problem: ") + replyJson.toJson(QJsonDocument::Indented));
        }
        reply->deleteLater();
    }
    deleteLater();
}

SetChannelTypeJob::GroupType SetChannelTypeJob::type() const
{
    return mType;
}

void SetChannelTypeJob::setType(GroupType type)
{
    mType = type;
}

QString SetChannelTypeJob::roomId() const
{
    return mRoomId;
}

void SetChannelTypeJob::setRoomId(const QString &roomId)
{
    mRoomId = roomId;
}

bool SetChannelTypeJob::requireHttpAuthentication() const
{
    return true;
}

bool SetChannelTypeJob::canStart() const
{
    if (mRoomId.isEmpty()) {
        qCWarning(ROCKETCHATQTRESTAPI_LOG) << "SetChannelTypeJob: mRoomId is empty";
        return false;
    }
    if (mType == Unknown) {
        qCWarning(ROCKETCHATQTRESTAPI_LOG) << "SetChannelTypeJob: type is not defined";
        return false;
    }
    if (!RestApiAbstractJob::canStart()) {
        qCWarning(ROCKETCHATQTRESTAPI_LOG) << "Impossible to start SetChannelTypeJob job";
        return false;
    }
    return true;
}

QJsonDocument SetChannelTypeJob::json() const
{
    QJsonObject jsonObj;
    jsonObj[QLatin1String("roomId")] = roomId();
    switch (mType) {
    case Public:
        jsonObj[QLatin1String("type")] = QStringLiteral("c");
        break;
    case Private:
        jsonObj[QLatin1String("type")] = QStringLiteral("p");
        break;
    case Unknown:
        break;
    }

    const QJsonDocument postData = QJsonDocument(jsonObj);
    return postData;
}

QNetworkRequest SetChannelTypeJob::request() const
{
    const QUrl url = mRestApiMethod->generateUrl(RestApiUtil::RestApiUrlType::ChannelsSetType);
    QNetworkRequest request(url);
    addAuthRawHeader(request);
    addRequestAttribute(request);
    return request;
}
