(() => {
    const $ = (sel) => document.querySelector(sel);
    const host = location.host;
    $('#host').textContent = host;

    const dot = $('#status-dot');
    const setDot = (ok) => {
        dot.style.background = ok ? '#38d39f' : '#f25f5c';
        dot.style.boxShadow = ok ? '0 0 10px #38d39f' : '0 0 10px #f25f5c';
        dot.title = ok ? 'connected' : 'disconnected';
    };

    // --- i18n (EN / ES) -------------------------------------------------------
    const i18n = {
        en: {
            title: "Fridge Controller",

            status_title: "Status",
            label_current_temp: "Current Temperature",
            label_setpoint: "Setpoint",
            label_threshold: "Threshold",
            label_freezing: "Freezing",
            label_defrost: "Defrost",

            temp_title: "Temperature Control",
            label_setpoint_input: "Setpoint (°C)",
            label_threshold_input: "Threshold (°C)",
            label_freezing_toggle: "Freezing",
            label_defrost_toggle: "Defrost",
            btn_setpoint_apply: "Apply Setpoint",
            btn_threshold_apply: "Apply Threshold",
            hint_temp_auth: "Changing these values or toggling freezing/defrost will require the admin password. Your browser will show a login popup the first time.",

            timers_title: "Defrost Timers",
            timers_label_index: "Timer index (0–9)",
            timers_label_start: "Start (HH:MM)",
            timers_label_stop: "Stop (HH:MM)",
            btn_timer_load: "Load Timer",
            btn_timer_save: "Save Timer",
            timers_hint: "Select a timer index and click “Load Timer” to see current values.",

            wifi_title: "WiFi & Security",
            wifi_label_ssid: "WiFi SSID",
            wifi_label_pass: "WiFi Password",
            wifi_label_ap_pass: "AP Password",
            wifi_label_ui_pass: "UI Password",
            btn_wifi_save: "Save & Reboot",
            btn_wifi_scan: "Scan Networks",
            btn_wifi_reset: "Clear Config",

            maint_title: "Maintenance",
            maint_text: "Upload new firmware via the built-in updater. You’ll be prompted for the admin user and password.",
            maint_btn: "Firmware Update",

            footer_text: "ESP8266 Fridge",

            // Button texts for dynamic modes
            btn_freeze_on: "Turn Freezing ON",
            btn_freeze_off: "Turn Freezing OFF",
            btn_defrost_on: "Turn Defrost ON",
            btn_defrost_off: "Turn Defrost OFF",
            label_loading: "Loading…",

            // Alerts / messages
            msg_need_setpoint: "Enter a valid setpoint temperature.",
            msg_setpoint_ok: "Setpoint updated.",
            msg_setpoint_err: "Error updating setpoint: ",

            msg_need_threshold: "Enter a valid threshold temperature.",
            msg_threshold_ok: "Threshold updated.",
            msg_threshold_err: "Error updating threshold: ",

            msg_timer_invalid: "Invalid timer index.",
            msg_timer_fields: "Please fill all timer fields with valid numbers.",
            msg_timer_range: "Hours must be 0–23 and minutes 0–59.",
            msg_timer_ok: "Timer saved.",
            msg_timer_load_err: "Error loading timer: ",
            msg_timer_save_err: "Error saving timer: ",

            msg_wifi_loading: "Loading WiFi configuration…",
            msg_wifi_none: "No WiFi configured yet.",
            msg_wifi_err: "Error loading WiFi config.",
            msg_wifi_scan_none: "No networks found.",
            msg_wifi_scan_err: "Error scanning WiFi: ",
            msg_wifi_saved: "Saved. Device will reboot and reconnect.",
            msg_wifi_save_err: "Error saving WiFi config: ",
            msg_wifi_reset_confirm: "Clear WiFi config and reboot?",
            msg_wifi_reset_ok: "WiFi config cleared. Device will reboot.",
            msg_wifi_reset_err: "Error clearing WiFi config: ",

            msg_auth_required: "Admin login required. Your browser will show a login popup.\nUse user \"admin\" and the UI password configured in the device.",
        },
        es: {
            title: "Control de refrigerador",

            status_title: "Estado",
            label_current_temp: "Temperatura actual",
            label_setpoint: "Setpoint",
            label_threshold: "Umbral",
            label_freezing: "Enfriamiento",
            label_defrost: "Deshielo",

            temp_title: "Control de temperatura",
            label_setpoint_input: "Setpoint (°C)",
            label_threshold_input: "Umbral (°C)",
            label_freezing_toggle: "Enfriamiento",
            label_defrost_toggle: "Deshielo",
            btn_setpoint_apply: "Aplicar setpoint",
            btn_threshold_apply: "Aplicar umbral",
            hint_temp_auth: "Cambiar estos valores o activar/desactivar enfriamiento/deshielo requerirá la contraseña de administrador. El navegador mostrará una ventana de inicio de sesión la primera vez.",

            timers_title: "Temporizadores de deshielo",
            timers_label_index: "Índice de temporizador (0–9)",
            timers_label_start: "Inicio (HH:MM)",
            timers_label_stop: "Fin (HH:MM)",
            btn_timer_load: "Cargar temporizador",
            btn_timer_save: "Guardar temporizador",
            timers_hint: "Selecciona un índice de temporizador y haz clic en “Cargar temporizador” para ver los valores actuales.",

            wifi_title: "WiFi y seguridad",
            wifi_label_ssid: "SSID de WiFi",
            wifi_label_pass: "Contraseña de WiFi",
            wifi_label_ap_pass: "Contraseña de AP",
            wifi_label_ui_pass: "Contraseña de interfaz",
            btn_wifi_save: "Guardar y reiniciar",
            btn_wifi_scan: "Buscar redes",
            btn_wifi_reset: "Borrar configuración",

            maint_title: "Mantenimiento",
            maint_text: "Sube nuevo firmware usando el actualizador. Se te pedirá el usuario y la contraseña de administrador.",
            maint_btn: "Actualizar firmware",

            footer_text: "Refrigerador ESP8266",

            btn_freeze_on: "Encender enfriamiento",
            btn_freeze_off: "Apagar enfriamiento",
            btn_defrost_on: "Encender deshielo",
            btn_defrost_off: "Apagar deshielo",
            label_loading: "Cargando…",

            msg_need_setpoint: "Ingresa un setpoint válido.",
            msg_setpoint_ok: "Setpoint actualizado.",
            msg_setpoint_err: "Error al actualizar el setpoint: ",

            msg_need_threshold: "Ingresa un umbral válido.",
            msg_threshold_ok: "Umbral actualizado.",
            msg_threshold_err: "Error al actualizar el umbral: ",

            msg_timer_invalid: "Índice de temporizador no válido.",
            msg_timer_fields: "Completa todos los campos del temporizador con números válidos.",
            msg_timer_range: "Las horas deben ser de 0–23 y los minutos de 0–59.",
            msg_timer_ok: "Temporizador guardado.",
            msg_timer_load_err: "Error al cargar el temporizador: ",
            msg_timer_save_err: "Error al guardar el temporizador: ",

            msg_wifi_loading: "Cargando configuración de WiFi…",
            msg_wifi_none: "Aún no hay WiFi configurada.",
            msg_wifi_err: "Error al cargar la configuración de WiFi.",
            msg_wifi_scan_none: "No se encontraron redes.",
            msg_wifi_scan_err: "Error al buscar redes WiFi: ",
            msg_wifi_saved: "Guardado. El dispositivo se reiniciará y se reconectará.",
            msg_wifi_save_err: "Error al guardar la configuración de WiFi: ",
            msg_wifi_reset_confirm: "¿Borrar configuración de WiFi y reiniciar?",
            msg_wifi_reset_ok: "Configuración de WiFi borrada. El dispositivo se reiniciará.",
            msg_wifi_reset_err: "Error al borrar la configuración de WiFi: ",

            msg_auth_required: "Se requiere inicio de sesión de administrador. El navegador mostrará una ventana de login.\nUsa el usuario \"admin\" y la contraseña de la interfaz configurada en el dispositivo.",
        }
    };

    let currentLang = localStorage.getItem('lang') || 'en';

    function t(key) {
        const dict = i18n[currentLang] || i18n.en;
        return dict[key] || i18n.en[key] || key;
    }

    function applyLanguage(lang) {
        currentLang = lang;
        localStorage.setItem('lang', lang);
        const dict = i18n[lang] || i18n.en;

        document.querySelectorAll('[data-i18n]').forEach(el => {
            const key = el.getAttribute('data-i18n');
            if (!key) return;
            const value = dict[key] || i18n.en[key];
            if (!value) return;
            el.textContent = value;
        });

        // Update <title>
        document.title = dict.title || i18n.en.title;

        // Set language selector UI
        const sel = document.getElementById('langSwitch');
        if (sel) sel.value = lang;
    }

    // Hook language selector
    const langSel = document.getElementById('langSwitch');
    if (langSel) {
        langSel.addEventListener('change', () => {
            applyLanguage(langSel.value);
        });
    }

    // --- Generic helpers ------------------------------------------------------

    async function getJSON(path) {
        const r = await fetch(path, { cache: 'no-store' });
        setDot(r.ok);
        if (!r.ok) {
            if (r.status === 401) {
                alert(t('msg_auth_required'));
            }
            let msg = 'Error';
            try { msg = await r.text(); } catch (e) { }
            throw new Error(msg || 'Request failed');
        }
        return r.json();
    }

    async function fetchRealIP() {
        try {
            const info = await getJSON('/ip');
            if (info && info.ip) {
                const hostSpan = document.getElementById('host');
                hostSpan.textContent = info.ip + " (" + location.host + ")";
            }
        } catch (e) {
            console.warn("Could not fetch real IP:", e);
        }
    }


    async function callText(path, options = {}) {
        const r = await fetch(path, options);
        setDot(r.ok);
        if (!r.ok) {
            if (r.status === 401) {
                alert(t('msg_auth_required'));
            }
            let msg = 'Error';
            try { msg = await r.text(); } catch (e) { }
            throw new Error(msg || 'Request failed');
        }
        try {
            return await r.text();
        } catch (e) {
            return '';
        }
    }

    async function postForm(path, params) {
        const body = new URLSearchParams(params);
        return callText(path, {
            method: 'POST',
            headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
            body
        });
    }

    // --- State -----------------------------------------------------------------
    const state = {
        freezing: null,
        defrost: null,
    };

    function updateModeUI() {
        const freezeLabel = $('#freezeState');
        const defrostLabel = $('#defrostState');
        const freezeBtn = $('#freezeToggleBtn');
        const defrostBtn = $('#defrostToggleBtn');

        if (state.freezing === null) {
            freezeLabel.textContent = 'UNKNOWN';
            freezeLabel.className = 'badge';
            freezeBtn.textContent = t('label_loading');
            freezeBtn.disabled = true;
        } else {
            freezeLabel.textContent = state.freezing ? 'ON' : 'OFF';
            freezeLabel.className = 'badge ' + (state.freezing ? 'badge-on' : 'badge-off');
            freezeBtn.textContent = state.freezing ? t('btn_freeze_off') : t('btn_freeze_on');
            freezeBtn.disabled = false;
        }

        if (state.defrost === null) {
            defrostLabel.textContent = 'UNKNOWN';
            defrostLabel.className = 'badge';
            defrostBtn.textContent = t('label_loading');
            defrostBtn.disabled = true;
        } else {
            defrostLabel.textContent = state.defrost ? 'ON' : 'OFF';
            defrostLabel.className = 'badge ' + (state.defrost ? 'badge-on' : 'badge-off');
            defrostBtn.textContent = state.defrost ? t('btn_defrost_off') : t('btn_defrost_on');
            defrostBtn.disabled = false;
        }
    }

    // --- Sensors / Status ------------------------------------------------------

    async function refresh() {
        try {
            const s = await getJSON('/sensors');

            const cur = (typeof s.currentTemp === 'number')
                ? s.currentTemp.toFixed(2)
                : s.currentTemp;
            const setp = (typeof s.setTemp === 'number')
                ? s.setTemp.toFixed(2)
                : s.setTemp;
            const thr = (typeof s.threshold === 'number')
                ? s.threshold.toFixed(2)
                : s.threshold;

            $('#currentTemp').textContent = `${cur} °C`;
            $('#setTemp').textContent = `${setp} °C`;
            $('#threshold').textContent = `${thr} °C`;

            if (typeof s.freezing !== 'undefined') {
                state.freezing = !!s.freezing;
            }
            if (typeof s.defrost !== 'undefined') {
                state.defrost = !!s.defrost;
            }
            updateModeUI();
        } catch (e) {
            console.error('refresh error', e);
        }
    }

    // --- Temperature controls --------------------------------------------------

    $('#setTempBtn').addEventListener('click', async () => {
        const v = parseFloat($('#setTempInput').value);
        if (Number.isNaN(v)) {
            alert(t('msg_need_setpoint'));
            return;
        }
        try {
            await callText(`/setFrTemp?temp=${encodeURIComponent(v)}`);
            alert(t('msg_setpoint_ok'));
            refresh();
        } catch (e) {
            alert(t('msg_setpoint_err') + e.message);
        }
    });

    $('#thresholdBtn').addEventListener('click', async () => {
        const v = parseFloat($('#thresholdInput').value);
        if (Number.isNaN(v)) {
            alert(t('msg_need_threshold'));
            return;
        }
        try {
            await callText(`/setTrhTemp?temp=${encodeURIComponent(v)}`);
            alert(t('msg_threshold_ok'));
            refresh();
        } catch (e) {
            alert(t('msg_threshold_err') + e.message);
        }
    });

    $('#freezeToggleBtn').addEventListener('click', async () => {
        try {
            const path = state.freezing ? '/freezeOff' : '/freezeOn';
            await callText(path);
            await refresh();
        } catch (e) {
            alert('Error: ' + e.message);
        }
    });

    $('#defrostToggleBtn').addEventListener('click', async () => {
        try {
            const path = state.defrost ? '/defrostOff' : '/defrostOn';
            await callText(path);
            await refresh();
        } catch (e) {
            alert('Error: ' + e.message);
        }
    });

    // --- Defrost timers --------------------------------------------------------

    $('#timerLoadBtn').addEventListener('click', async () => {
        const idx = parseInt($('#timerIndex').value, 10);
        if (Number.isNaN(idx)) {
            alert(t('msg_timer_invalid'));
            return;
        }
        try {
            const tmr = await getJSON(`/timersGet?index=${idx}`);

            $('#startHour').value = tmr.startHours ?? 0;
            $('#startMinute').value = tmr.startMinutes ?? 0;
            $('#stopHour').value = tmr.stopHours ?? 0;
            $('#stopMinute').value = tmr.stopMinutes ?? 0;

            const hStart = tmr.startHours ?? '--';
            const mStart = (tmr.startMinutes ?? '--').toString().padStart(2, '0');
            const hStop = tmr.stopHours ?? '--';
            const mStop = (tmr.stopMinutes ?? '--').toString().padStart(2, '0');

            $('#timerInfo').textContent = currentLang === 'es'
                ? `Temporizador ${idx}: de ${hStart}:${mStart} a ${hStop}:${mStop}.`
                : `Timer ${idx}: from ${hStart}:${mStart} to ${hStop}:${mStop}.`;

        } catch (e) {
            alert(t('msg_timer_load_err') + e.message);
        }
    });

    $('#timerSaveBtn').addEventListener('click', async () => {
        const idx = parseInt($('#timerIndex').value, 10);
        const shour = parseInt($('#startHour').value, 10);
        const smin = parseInt($('#startMinute').value, 10);
        const ehour = parseInt($('#stopHour').value, 10);
        const emin = parseInt($('#stopMinute').value, 10);

        if ([idx, shour, smin, ehour, emin].some(Number.isNaN)) {
            alert(t('msg_timer_fields'));
            return;
        }
        if (shour < 0 || shour > 23 || ehour < 0 || ehour > 23 ||
            smin < 0 || smin > 59 || emin < 0 || emin > 59) {
            alert(t('msg_timer_range'));
            return;
        }

        const qs = new URLSearchParams({
            index: idx,
            startHour: shour,
            startMinute: smin,
            stopHour: ehour,
            stopMinute: emin
        }).toString();

        try {
            await callText(`/timersDef?${qs}`);
            alert(t('msg_timer_ok'));
        } catch (e) {
            alert(t('msg_timer_save_err') + e.message);
        }
    });

    // --- WiFi configuration ----------------------------------------------------

    async function loadWifi() {
        try {
            const w = await getJSON('/wifiGet');
            $('#wifiSsid').value = w.ssid || '';
            $('#wifiPass').value = ''; // never show real password
            if (w.ssid) {
                $('#wifiStatus').textContent = currentLang === 'es'
                    ? `WiFi actual: ${w.ssid}`
                    : `Current WiFi: ${w.ssid}`;
            } else {
                $('#wifiStatus').textContent = t('msg_wifi_none');
            }
        } catch (e) {
            $('#wifiStatus').textContent = t('msg_wifi_err');
            console.error('wifiGet error', e);
        }
    }

    $('#wifiSave').addEventListener('click', async () => {
        const ssid = $('#wifiSsid').value.trim();
        const pass = $('#wifiPass').value.trim();
        const apPass = $('#apPass').value.trim();
        const uiPass = $('#uiPass').value.trim();

        if (!ssid) {
            // same message in both languages is fine here; optional:
            const msg = currentLang === 'es'
                ? 'El SSID está vacío. Esto deshabilitará el modo STA. ¿Continuar?'
                : 'SSID is empty. This will disable STA mode. Continue?';
            if (!confirm(msg)) return;
        }

        const params = { ssid, pass };
        if (apPass) params.apPass = apPass;
        if (uiPass) params.uiPass = uiPass;

        try {
            await postForm('/wifiSet', params);
            alert(t('msg_wifi_saved'));
        } catch (e) {
            alert(t('msg_wifi_save_err') + e.message);
        }
    });

    $('#wifiScan').addEventListener('click', async () => {
        try {
            const list = await getJSON('/wifiScan');
            const box = $('#wifiList');
            box.innerHTML = '';
            if (!Array.isArray(list) || list.length === 0) {
                box.textContent = t('msg_wifi_scan_none');
                return;
            }
            list.forEach(n => {
                const d = document.createElement('div');
                d.className = 'wifi-item';
                d.innerHTML = `<strong>${n.ssid}</strong><span>RSSI ${n.rssi} dBm${n.open ? ' (open)' : ''}</span>`;
                d.onclick = () => { $('#wifiSsid').value = n.ssid; };
                box.appendChild(d);
            });
        } catch (e) {
            alert(t('msg_wifi_scan_err') + e.message);
        }
    });

    $('#wifiReset').addEventListener('click', async () => {
        if (!confirm(t('msg_wifi_reset_confirm'))) return;
        try {
            await callText('/wifiReset');
            alert(t('msg_wifi_reset_ok'));
        } catch (e) {
            alert(t('msg_wifi_reset_err') + e.message);
        }
    });

    //Graphs and data
    async function drawSimpleTempChart() {
        const canvas = document.getElementById('tempChart');
        if (!canvas || !canvas.getContext) return;
        const ctx = canvas.getContext('2d');

        // Pedir CSV
        const resp = await fetch('/logs/temp.csv', { cache: 'no-store' });
        if (!resp.ok) return;
        const text = await resp.text();

        // Parsear líneas (saltando encabezado)
        const lines = text.trim().split('\n').slice(1);
        const temps = lines.map(l => {
            const parts = l.split(',');
            return parseFloat(parts[2]); // dayIndex,hour,tempC
        }).filter(v => !Number.isNaN(v));

        if (temps.length === 0) return;

        // Preparamos canvas
        const w = canvas.width = canvas.clientWidth || 300;
        const h = canvas.height = 200;

        const minT = Math.min(...temps);
        const maxT = Math.max(...temps);
        const span = (maxT - minT) || 1;

        ctx.clearRect(0, 0, w, h);
        ctx.beginPath();

        temps.forEach((t, i) => {
            const x = (i / Math.max(temps.length - 1, 1)) * (w - 20) + 10;
            const y = h - ((t - minT) / span) * (h - 20) - 10;
            if (i === 0) ctx.moveTo(x, y);
            else ctx.lineTo(x, y);
        });

        ctx.stroke();
    }

    // Botones de descarga
    document.getElementById('downloadTemp').addEventListener('click', () => {
        window.location.href = '/logs/temp.csv';
    });
    document.getElementById('downloadFreeze').addEventListener('click', () => {
        window.location.href = '/logs/freeze.csv';
    });


    // --- Init ------------------------------------------------------------------

    applyLanguage(currentLang);
    refresh();
    loadWifi();
    drawSimpleTempChart();
    setInterval(refresh, 5000);
    fetchRealIP();
    setInterval(refresh, 5000);
})();
