package org.mozilla.vrbrowser.browser.extensions;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import org.json.JSONObject;
import org.mozilla.geckoview.GeckoResult;
import org.mozilla.geckoview.GeckoSession;
import org.mozilla.geckoview.WebExtensionController;
import org.mozilla.vrbrowser.browser.engine.Session;

import java.util.HashMap;
import java.util.Map;
import java.util.stream.Collectors;
import java.util.stream.Stream;

import mozilla.components.concept.engine.webextension.ActionHandler;
import mozilla.components.concept.engine.webextension.MessageHandler;
import mozilla.components.concept.engine.webextension.Metadata;
import mozilla.components.concept.engine.webextension.Port;
import mozilla.components.concept.engine.webextension.WebExtension;
import mozilla.components.concept.engine.webextension.WebExtensionEngineSession;

public class GeckoWebExtension extends WebExtension {

    private org.mozilla.geckoview.WebExtension mNativeExtension;
    private Map<PortId, Port> mConnectedPorts = new HashMap<>();

    class PortId {

        public String name;
        public WebExtensionEngineSession engineSession;

        PortId(@NonNull String name, @Nullable WebExtensionEngineSession engineSession) {
            this.name = name;
            this.engineSession = engineSession;
        }

        PortId(@NonNull String name) {
            this(name, null);
        }

        @Override
        public int hashCode() {
            return name.hashCode();
        }

        @Override
        public boolean equals(@Nullable Object obj) {
            if (obj instanceof PortId) {
                return ((PortId)obj).name.equals(name) && (((PortId)obj).engineSession == engineSession);
            }

            return false;
        }

        @NonNull
        @Override
        public String toString() {
            return "PortId(name=" + name + ", engineSession=" + engineSession.toString() + ")";
        }
    }

    /**
     * Represents a browser extension based on the WebExtension API:
     * https://developer.mozilla.org/en-US/docs/Mozilla/Add-ons/WebExtensions
     *
     * @param id  the unique ID of this extension.
     * @param url the url pointing to a resources path for locating the extension
     */
    public GeckoWebExtension(@NonNull String id,
                             @NonNull String url,
                             WebExtensionController webExtensionController,
                             boolean allowContentMessaging) {
        super(id, url, allowContentMessaging);


        mNativeExtension = new org.mozilla.geckoview.WebExtension(url,
                id,
                createWebExtensionFlags(allowContentMessaging),
                webExtensionController);
    }

    public org.mozilla.geckoview.WebExtension getNativeExtension() {
        return mNativeExtension;
    }

    /**
     * See [WebExtension.registerBackgroundMessageHandler].
     */
    @Override
    public void registerBackgroundMessageHandler(@NonNull String name, @NonNull MessageHandler messageHandler) {
        org.mozilla.geckoview.WebExtension.PortDelegate portDelegate = new org.mozilla.geckoview.WebExtension.PortDelegate() {
            @Override
            public void onPortMessage(@NonNull Object message, @NonNull org.mozilla.geckoview.WebExtension.Port port) {
                messageHandler.onPortMessage(message, new GeckoPort(port));
            }

            @NonNull
            @Override
            public void onDisconnect(@NonNull org.mozilla.geckoview.WebExtension.Port port) {
                mConnectedPorts.remove(new PortId(name));
                messageHandler.onPortDisconnected(new GeckoPort(port));
            }
        };

        org.mozilla.geckoview.WebExtension.MessageDelegate messageDelegate = new org.mozilla.geckoview.WebExtension.MessageDelegate() {

            @Nullable
            @Override
            public void onConnect(@NonNull org.mozilla.geckoview.WebExtension.Port port) {
                port.setDelegate(portDelegate);
                GeckoPort geckoPort = new GeckoPort(port);
                mConnectedPorts.put(new PortId(name),  geckoPort);
                messageHandler.onPortConnected(geckoPort);
            }

            @Nullable
            @Override
            public GeckoResult<Object> onMessage(@NonNull String name, @NonNull Object message, @NonNull org.mozilla.geckoview.WebExtension.MessageSender messageSender) {
                Object response = messageHandler.onMessage(message, null);
                return GeckoResult.fromValue(response);
            }
        };

        mNativeExtension.setMessageDelegate(messageDelegate, name);
    }

    /**
     * See [WebExtension.registerContentMessageHandler].
     */
    @Override
    public void registerContentMessageHandler(@NonNull WebExtensionEngineSession session, @NonNull String name, @NonNull MessageHandler messageHandler) {
        org.mozilla.geckoview.WebExtension.PortDelegate portDelegate = new org.mozilla.geckoview.WebExtension.PortDelegate() {
            @Override
            public void onPortMessage(@NonNull Object message, @NonNull org.mozilla.geckoview.WebExtension.Port port) {
                messageHandler.onPortMessage(message, new GeckoPort(port, session));
            }

            @NonNull
            @Override
            public void onDisconnect(@NonNull org.mozilla.geckoview.WebExtension.Port port) {
                mConnectedPorts.remove(new PortId(name, session));
                messageHandler.onPortDisconnected(new GeckoPort(port, session));
            }
        };

        org.mozilla.geckoview.WebExtension.MessageDelegate messageDelegate = new org.mozilla.geckoview.WebExtension.MessageDelegate() {

            @Nullable
            @Override
            public void onConnect(@NonNull org.mozilla.geckoview.WebExtension.Port port) {
                port.setDelegate(portDelegate);
                GeckoPort geckoPort = new GeckoPort(port,session);
                mConnectedPorts.put(new PortId(name, session),  geckoPort);
                messageHandler.onPortConnected(geckoPort);
            }

            @Nullable
            @Override
            public GeckoResult<Object> onMessage(@NonNull String name, @NonNull Object message, @NonNull org.mozilla.geckoview.WebExtension.MessageSender messageSender) {
                Object response = messageHandler.onMessage(message, session);
                return GeckoResult.fromValue(response);
            }
        };

        GeckoSession geckoSession = ((Session)session).getGeckoSession();
        geckoSession.setMessageDelegate(mNativeExtension, messageDelegate, name);
    }

    @Override
    public boolean hasContentMessageHandler(@NonNull WebExtensionEngineSession session, @NonNull String name) {
        GeckoSession geckoSession = ((Session)session).getGeckoSession();
        return geckoSession.getMessageDelegate(mNativeExtension, name) != null;
    }

    @Nullable
    @Override
    public Port getConnectedPort(@NonNull String name, @Nullable WebExtensionEngineSession session) {
        return mConnectedPorts.get(new PortId(name, session));
    }

    @Override
    public void disconnectPort(@NonNull String name, @Nullable WebExtensionEngineSession session) {
        PortId portId = new PortId(name, session);
        Port port = mConnectedPorts.get(portId);
        if (port != null) {
            port.disconnect();
            mConnectedPorts.remove(portId);
        }
    }

    @Override
    public void registerActionHandler(@NonNull WebExtensionEngineSession engineSession, @NonNull ActionHandler actionHandler) {
        // TODO
    }

    @Override
    public void registerActionHandler(@NonNull ActionHandler actionHandler) {
        // TODO
    }

    @Override
    public boolean hasActionHandler(@NonNull WebExtensionEngineSession engineSession) {
        return false;
    }

    @Override
    public Metadata getMetadata() {
        if (mNativeExtension.metaData != null) {
            return new Metadata(
                    mNativeExtension.metaData.version,
                    Stream.of(mNativeExtension.metaData.permissions).collect(Collectors.toList()),
                    Stream.of(mNativeExtension.metaData.origins).collect(Collectors.toList()),
                    mNativeExtension.metaData.name,
                    mNativeExtension.metaData.description,
                    mNativeExtension.metaData.creatorName,
                    mNativeExtension.metaData.creatorUrl,
                    mNativeExtension.metaData.homepageUrl,
                    null, // TODO
                    null,  // TODO
                    null // TODO
            );
        }

        return null;
    }

    @Override
    public boolean isEnabled() {
        return true;
    }

    private long createWebExtensionFlags(boolean allowContentMessaging) {
        if (allowContentMessaging) {
            return org.mozilla.geckoview.WebExtension.Flags.ALLOW_CONTENT_MESSAGING;

        } else {
            return org.mozilla.geckoview.WebExtension.Flags.NONE;
        }
    }

    /**
     * Gecko-based implementation of [Port], wrapping the native port provided by GeckoView.
     */
    class GeckoPort extends Port {

        private org.mozilla.geckoview.WebExtension.Port mNativePort;

        GeckoPort(@NonNull org.mozilla.geckoview.WebExtension.Port port,
                  @Nullable WebExtensionEngineSession session) {
            super(session);

            mNativePort = port;
        }

        GeckoPort(@NonNull org.mozilla.geckoview.WebExtension.Port port) {
            this(port, null);
        }

        public void postMessage(@NonNull JSONObject message) {
            mNativePort.postMessage(message);
        }

        public String name() {
            return mNativePort.name;
        }

        public void disconnect() {
            mNativePort.disconnect();
        }

    }
}
