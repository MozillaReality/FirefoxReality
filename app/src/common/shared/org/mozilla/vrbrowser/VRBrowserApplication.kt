/* -*- Mode: Java; c-basic-offset: 4; tab-width: 4; indent-tabs-mode: nil; -*-
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.vrbrowser

import android.app.Application
import mozilla.components.browser.engine.gecko.fetch.GeckoViewFetchClient

import org.mozilla.vrbrowser.browser.Places
import org.mozilla.vrbrowser.browser.Services
import org.mozilla.vrbrowser.db.AppDatabase
import org.mozilla.vrbrowser.telemetry.TelemetryWrapper

import mozilla.components.support.base.log.Log
import mozilla.components.support.base.log.sink.AndroidLogSink
import mozilla.components.support.rusthttp.RustHttpConfig
import mozilla.components.support.rustlog.RustLog
import org.mozilla.geckoview.GeckoRuntime

class VRBrowserApplication : Application() {

    private var appExecutors: AppExecutors? = null
    lateinit var services: Services
        private set
    lateinit var places: Places
        private set

    private val database: AppDatabase
        get() = AppDatabase.getInstance(this, appExecutors)

    val repository: DataRepository
        get() = DataRepository.getInstance(database, appExecutors)

    override fun onCreate() {
        super.onCreate()

        // Enable android-components and application-services logging.
        Log.addSink(AndroidLogSink())
        RustLog.enable()

        // Specify network stack to be used by application-services libraries.
        RustHttpConfig.setClient(lazy { GeckoViewFetchClient(this, GeckoRuntime()) })

        appExecutors = AppExecutors()
        places = Places(this)
        services = Services(this, places)

        TelemetryWrapper.init(this)
    }
}
