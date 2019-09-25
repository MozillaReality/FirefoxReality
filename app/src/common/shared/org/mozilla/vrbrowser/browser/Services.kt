/* -*- Mode: Java; c-basic-offset: 4; tab-width: 4; indent-tabs-mode: nil; -*-
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.vrbrowser.browser

import android.content.Context
import android.net.Uri
import android.os.Build
import androidx.lifecycle.ProcessLifecycleOwner
import androidx.work.Configuration
import androidx.work.WorkManager
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import mozilla.components.concept.sync.DeviceCapability
import mozilla.components.concept.sync.DeviceEvent
import mozilla.components.concept.sync.DeviceEventsObserver
import mozilla.components.concept.sync.DeviceType
import mozilla.components.service.fxa.DeviceConfig
import mozilla.components.service.fxa.ServerConfig
import mozilla.components.service.fxa.SyncConfig
import mozilla.components.service.fxa.manager.FxaAccountManager
import mozilla.components.service.fxa.sync.GlobalSyncableStoreProvider
import mozilla.components.support.base.log.Log
import mozilla.components.support.base.log.logger.Logger
import mozilla.components.support.base.log.sink.AndroidLogSink
import org.mozilla.vrbrowser.browser.engine.SessionStore
import java.lang.IllegalStateException

class Services(context: Context, places: Places) {
    companion object {
        // TODO this is from a sample app, get a real client id before shipping.
        const val CLIENT_ID = "3c49430b43dfba77"
        const val REDIRECT_URL = "https://accounts.firefox.com/oauth/success/$CLIENT_ID"
    }

    // This makes bookmarks storage accessible to background sync workers.
    init {
        // Make sure we get logs out of our android-components.
        Log.addSink(AndroidLogSink())

        GlobalSyncableStoreProvider.configureStore("bookmarks" to places.bookmarks)
        GlobalSyncableStoreProvider.configureStore("history" to places.history)

        // TODO this really shouldn't be necessary, since WorkManager auto-initializes itself, unless
        // auto-initialization is disabled in the manifest file. We don't disable the initialization,
        // but i'm seeing crashes locally because WorkManager isn't initialized correctly...
        // Maybe this is a race of sorts? We're trying to access it before it had a chance to auto-initialize?
        // It's not well-documented _when_ that auto-initialization is supposed to happen.

        // For now, let's just manually initialize it here, and swallow failures (it's already initialized).
        try {
            WorkManager.initialize(
                    context,
                    Configuration.Builder().setMinimumLoggingLevel(android.util.Log.INFO).build()
            )
        } catch (e: IllegalStateException) {}
    }

    // Process received device events, only handling received tabs for now.
    // They'll come from other FxA devices (e.g. Firefox Desktop).
    private val deviceEventObserver = object : DeviceEventsObserver {
        private val logTag = "DeviceEventsObserver"

        override fun onEvents(events: List<DeviceEvent>) {
            CoroutineScope(Dispatchers.Main).launch {
                Logger(logTag).info("Received ${events.size} device event(s)")
                events.filterIsInstance(DeviceEvent.TabReceived::class.java).forEach {
                    // Just load the first tab that was sent.
                    // TODO is there a notifications API of sorts here?
                    SessionStore.get().activeStore.loadUri(it.entries[0].url)
                }
            }
        }
    }

    val accountManager = FxaAccountManager(
        context = context,
        serverConfig = ServerConfig.release(CLIENT_ID, REDIRECT_URL),
        deviceConfig = DeviceConfig(
            // This is a default name, and can be changed once user is logged in.
            // E.g. accountManager.authenticatedAccount()?.deviceConstellation()?.setDeviceNameAsync("new name")
            name = "Firefox Reality on ${Build.MANUFACTURER} ${Build.MODEL}",
            // TODO need a new device type! "VR"
            type = DeviceType.MOBILE,
            capabilities = setOf(DeviceCapability.SEND_TAB)
        ),
        // If background syncing is desired, pass in a 'syncPeriodInMinutes' parameter.
        // As-is, sync will run on app startup.
        syncConfig = SyncConfig(setOf("bookmarks", "history"))
    ).also {
        it.registerForDeviceEvents(deviceEventObserver, ProcessLifecycleOwner.get(), true)
    }

    init {
        CoroutineScope(Dispatchers.Main).launch {
            accountManager.initAsync().await()
        }
    }

    /**
     * Call this for every loaded URL to enable FxA sign-in to finish. It's a bit of a hack, but oh well.
     */
    fun interceptFxaUrl(uri: String) {
        if (!uri.startsWith(REDIRECT_URL)) return
        val parsedUri = Uri.parse(uri)

        parsedUri.getQueryParameter("code")?.let { code ->
            val state = parsedUri.getQueryParameter("state") as String

            // Notify the state machine about our success.
            accountManager.finishAuthenticationAsync(code, state)
        }
    }
}