/* -*- Mode: Java; c-basic-offset: 4; tab-width: 4; indent-tabs-mode: nil; -*-
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.vrbrowser.ui.widgets.settings

import android.content.Context
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.future.future
import mozilla.components.concept.sync.AccountObserver
import mozilla.components.concept.sync.OAuthAccount
import mozilla.components.concept.sync.Profile
import org.mozilla.vrbrowser.VRBrowserApplication
import org.mozilla.vrbrowser.browser.Services
import java.util.concurrent.CompletableFuture

class AccountsHelper(private val settingsWidget: SettingsWidget) {
    val accountObserver = object : AccountObserver {
        override fun onAuthenticated(account: OAuthAccount) {
            settingsWidget.updateCurrentAccountState()
        }

        override fun onAuthenticationProblems() {
            settingsWidget.updateCurrentAccountState()
        }

        override fun onLoggedOut() {
            settingsWidget.updateCurrentAccountState()
        }

        override fun onProfileUpdated(profile: Profile) {
            settingsWidget.updateCurrentAccountState()
        }
    }

    fun authUrlAsync(): CompletableFuture<String?>? {
        val context = settingsWidget.context ?: return null

        return CoroutineScope(Dispatchers.Main).future {
            context.services().accountManager.beginAuthenticationAsync().await()
        }
    }

    private fun Context.services(): Services {
        return (this.applicationContext as VRBrowserApplication).services
    }
}