/* -*- Mode: Java; c-basic-offset: 4; tab-width: 4; indent-tabs-mode: nil; -*-
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.vrbrowser.telemetry;

import android.content.Context;

import org.mozilla.telemetry.Telemetry;
import org.mozilla.telemetry.TelemetryHolder;
import org.mozilla.telemetry.config.TelemetryConfiguration;
import org.mozilla.telemetry.measurement.DefaultSearchMeasurement;
import org.mozilla.telemetry.net.HttpURLConnectionTelemetryClient;
import org.mozilla.telemetry.net.TelemetryClient;
import org.mozilla.telemetry.ping.TelemetryCorePingBuilder;
import org.mozilla.telemetry.schedule.TelemetryScheduler;
import org.mozilla.telemetry.schedule.jobscheduler.JobSchedulerTelemetryScheduler;
import org.mozilla.telemetry.serialize.JSONPingSerializer;
import org.mozilla.telemetry.serialize.TelemetryPingSerializer;
import org.mozilla.telemetry.storage.FileTelemetryStorage;
import org.mozilla.telemetry.storage.TelemetryStorage;
import org.mozilla.vrbrowser.BuildConfig;

public class TelemetryWrapper {
    private static final String APP_NAME = "FirefoxReality";

    public static void init(Context context) {
        final TelemetryConfiguration configuration = new TelemetryConfiguration(context)
                .setAppName(APP_NAME)
                .setUpdateChannel(BuildConfig.BUILD_TYPE)
                .setBuildId(BuildConfig.VERSION_CODE + "-" + BuildConfig.FLAVOR);

        final TelemetryPingSerializer serializer = new JSONPingSerializer();
        final TelemetryStorage storage = new FileTelemetryStorage(configuration, serializer);
        final TelemetryClient client = new HttpURLConnectionTelemetryClient();
        final TelemetryScheduler scheduler = new JobSchedulerTelemetryScheduler();

        TelemetryHolder.set(new Telemetry(configuration, storage, client, scheduler)
                .addPingBuilder(new TelemetryCorePingBuilder(configuration))
                .setDefaultSearchProvider(createDefaultSearchEngineProvider()));
    }

    private static DefaultSearchMeasurement.DefaultSearchEngineProvider createDefaultSearchEngineProvider() {
        return new DefaultSearchMeasurement.DefaultSearchEngineProvider() {
            @Override
            public String getDefaultSearchEngineIdentifier() {
                return "google";
            }
        };
    }

    public static void start() {
        TelemetryHolder.get().recordSessionStart();
    }

    public static void stop() {
        TelemetryHolder.get().recordSessionEnd();

        TelemetryHolder.get()
                .queuePing(TelemetryCorePingBuilder.TYPE)
                .scheduleUpload();
    }
}
