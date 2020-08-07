package org.mozilla.vrbrowser.browser

import android.content.Context
import android.util.Log
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.GlobalScope
import kotlinx.coroutines.future.future
import kotlinx.coroutines.launch
import mozilla.components.concept.storage.Login
import mozilla.components.lib.dataprotect.SecureAbove22Preferences
import mozilla.components.lib.dataprotect.generateEncryptionKey
import mozilla.components.service.fxa.SyncEngine
import mozilla.components.service.fxa.sync.GlobalSyncableStoreProvider
import mozilla.components.service.sync.logins.GeckoLoginStorageDelegate
import mozilla.components.service.sync.logins.SyncableLoginsStorage
import org.mozilla.vrbrowser.browser.components.GeckoLoginDelegateWrapper
import org.mozilla.vrbrowser.browser.engine.EngineProvider
import java.util.concurrent.CompletableFuture

class LoginStorage(
        val context: Context
) {
    /**
     * Shared Preferences that encrypt/decrypt using Android KeyStore and lib-dataprotect for 23+
     * only on Nightly/Debug for now, otherwise simply stored.
     * See https://github.com/mozilla-mobile/fenix/issues/8324
     */
    private fun getSecureAbove22Preferences() =
            SecureAbove22Preferences(
                    context = context,
                    name = KEY_STORAGE_NAME
            )

    private val passwordsEncryptionKey by lazy {
        getSecureAbove22Preferences().getString(PASSWORDS_KEY)
                ?: generateEncryptionKey(KEY_STRENGTH).also {
                    if (SettingsStore.getInstance(context).isPasswordsEncryptionKeyGenerated) {
                        // We already had previously generated an encryption key, but we have lost it
                        Log.d(LOG_TAG,"Passwords encryption key for passwords storage was lost and we generated a new one")
                    }
                    SettingsStore.getInstance(context).recordPasswordsEncryptionKeyGenerated()
                    getSecureAbove22Preferences().putString(PASSWORDS_KEY, it)
                }
    }

    val lazyPasswordsStorage = lazy { SyncableLoginsStorage(context, passwordsEncryptionKey) }

    val passwordsStorage by lazy { lazyPasswordsStorage.value }

    init {
        EngineProvider.getOrCreateRuntime(context).loginStorageDelegate = GeckoLoginDelegateWrapper(
                GeckoLoginStorageDelegate(
                        lazyPasswordsStorage,
                        isAutofillEnabled = { SettingsStore.getInstance(context).isAutoFillEnabled }
                ))
        GlobalScope.launch(Dispatchers.IO) {
            passwordsStorage.warmUp()
        }

        GlobalSyncableStoreProvider.configureStore(SyncEngine.Passwords to lazyPasswordsStorage)
    }

    companion object {
        private const val LOG_TAG = "LoginStorage"
        private const val KEY_STRENGTH = 256
        private const val KEY_STORAGE_NAME = "fxr_secure_prefs"
        private const val PASSWORDS_KEY = "passwords"
    }

    fun getLogins(): CompletableFuture<List<Login>> = GlobalScope.future {
        lazyPasswordsStorage.value.list()
    }

    fun deleteEverything() = GlobalScope.future {
        lazyPasswordsStorage.value.wipeLocal()
    }

    fun delete(login: Login) = GlobalScope.future {
        lazyPasswordsStorage.value.delete(login.guid!!);
    }

    fun update(login: Login) = GlobalScope.future {
        lazyPasswordsStorage.value.update(login);
    }

}