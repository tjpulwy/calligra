/* This file is part of the KDE project
 * Copyright (C) 2014 Stuart Dickson <stuartmd@kogmbh.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kritasteamclient.h"

#include <QCoreApplication>
#include <QDebug>
#include <QMessageBox>
#include <QTimer>
#include <unistd.h>
#include "steam/steam_api.h"

#include "kis_steamcloudstorage.h"
// Can test if SteamTenfoot=1
#define BIGPICTURE_ENVVARNAME "SteamTenfoot"


KritaSteamClient *KritaSteamClient::sm_instance = 0;
KritaSteamClient* KritaSteamClient::instance()
{
    if (!sm_instance) {
        sm_instance = new KritaSteamClient(QCoreApplication::instance());
    }
    return sm_instance;
}

//-----------------------------------------------------------------------------
// Callback hook for debug text emitted from the Steam API
//-----------------------------------------------------------------------------
extern "C" void __cdecl SteamAPIDebugTextHook( int nSeverity, const char *pchDebugText )
{
    // if you're running in the debugger, only warnings (nSeverity >= 1) will be sent
    // if you add -debug_steamapi to the command-line, a lot of extra informational messages will also be sent
    qDebug( pchDebugText );

    if ( nSeverity >= 1 )
    {
        // place to set a breakpoint for catching API errors
        int x = 3;
        x = x;
    }
}

//-----------------------------------------------------------------------------
// Purpose: Wrapper around SteamAPI_WriteMiniDump which can be used directly
// as a se translator
//-----------------------------------------------------------------------------
void KritaSteamClient::MiniDumpFunction(const QString& comment, unsigned int nExceptionCode, void *pException )
{
    SteamAPI_SetMiniDumpComment( comment.toUtf8().constData() );

    QMessageBox msg(0);
    msg.setText("Caught error (Steam)");
    msg.setModal(true);
    msg.show();
    // The 0 here is a build ID, which is not set
#ifdef Q_OS_WIN
    SteamAPI_WriteMiniDump( nExceptionCode, (EXCEPTION_POINTERS*) pException, 0 );
#endif
}

class KritaSteamClient::Private
{
public:
    Private(KritaSteamClient* qq)
    : appId(0)
      , initialisedWithoutError(false)
      , bigPictureMode(false)
      , callbackTimer(0)
      , remoteStorage(0)
    {
    }

    uint32 appId;
    bool initialisedWithoutError;
    bool bigPictureMode;
    QTimer* callbackTimer;
    KisSteamCloudStorage* remoteStorage;
};

KritaSteamClient::KritaSteamClient(QObject *parent)
    : QObject(parent)
    , m_gameOverlayActivatedCallback( this, &KritaSteamClient::onGameOverlayActivated )
    , m_steamShutdownCallback( this, &KritaSteamClient::onSteamShutdown )
    , d( new Private(this) )
{
}


KritaSteamClient::~KritaSteamClient()
{
}

/**
 * @brief Launches the Steam Client to launch the app from Steam
 * @param appId The Steam-assigned app-id
 * @return true if Steam will launch the app, false otherwise
 */
bool KritaSteamClient::restartInSteam() {
    if (d->appId != k_uAppIdInvalid) {
        if (SteamAPI_RestartAppIfNecessary(d->appId))
        {
            // if Steam isn't running or Krita is not launched from Steam, this
            // starts the local client and launches it again.
            qDebug("Starting the local Steam client and launching through Steam");
            return true;
        }
    }
    return false;
}

bool KritaSteamClient::initialise(uint32 appId)
{
    const char* steamLanguage = 0;
    d->appId = appId;

    /*
     * This is where we used to call restartInSteam()
     * This has been removed in order to allow Krita
     * to continue to function without Steam running,
     * instead it displays a warning.
     */

    // Check for Big Picture mode
    QByteArray bigPictureEnvVar = qgetenv(BIGPICTURE_ENVVARNAME);
    if (!bigPictureEnvVar.isEmpty()) {
        d->bigPictureMode = (bigPictureEnvVar.toInt() == 1);
    }
    if (d->bigPictureMode) {
        qDebug("Steam is in BIG PICTURE mode");
    } else {
        qDebug("Steam is not in BIG PICTURE mode");
    }


    d->initialisedWithoutError = SteamAPI_Init();
    if (d->initialisedWithoutError) {
        qDebug("Steam initialised correctly");

        // set our debug handler
        SteamClient()->SetWarningMessageHook( &SteamAPIDebugTextHook );

        // Get Steam Language
        steamLanguage = SteamUtils()->GetSteamUILanguage();

        // Print Debug info
        qDebug() << QString("Steam UI language: ") << QString(steamLanguage);

        d->remoteStorage = new KisSteamCloudStorage();
    } else {
        qDebug("Steam did not initialise correctly!");
    }
    return true;
}

void KritaSteamClient::shutdown()
{
    if(d->initialisedWithoutError) {
        //SteamController()->Shutdown();
        SteamAPI_Shutdown();
    }
}

bool KritaSteamClient::isInBigPictureMode() {
    return d->bigPictureMode;
}

// Slots
void KritaSteamClient::runCallbacks()
{
    // Call the SteamAPI callback mechanism
    // This should be at >10Hz
    SteamAPI_RunCallbacks();
}

void KritaSteamClient::mainWindowCreated()
{
    if (d->initialisedWithoutError) {
        // Register >10Hz timer to process SteamAPI callback mechanism
        d->callbackTimer = new QTimer(this);
        d->callbackTimer->setInterval(100); // 10Hz
        d->callbackTimer->setSingleShot(false);
        connect(d->callbackTimer, SIGNAL(timeout()), this, SLOT(runCallbacks()));
        d->callbackTimer->start();
    }
}

void KritaSteamClient::mainWindowBeingDestroyed()
{
    if (d->initialisedWithoutError) {
        if (d->callbackTimer != 0) {
            d->callbackTimer->disconnect();
            d->callbackTimer->stop();
            delete d->callbackTimer;
            d->callbackTimer = 0;
        }
    }
}


// Steam Callbacks

void KritaSteamClient::onGameOverlayActivated( GameOverlayActivated_t *callback )
{
    if (callback->m_bActive) {
        qDebug("Emitting OverlayActivated");
        emit(overlayActivated());
    } else {
        qDebug("Emitting OverlayDeactivated");
        emit(overlayDeactivated());
    }
}

void KritaSteamClient::onSteamShutdown( SteamShutdown_t *callback )
{
    qDebug("Steam shutdown request, TODO: shutdown");
}

bool KritaSteamClient::isInitialised()
{
    return d->initialisedWithoutError;
}


KisSteamCloudStorage* KritaSteamClient::remoteStorage() {
    return d->remoteStorage;
}

#include "kritasteamclient.moc"
