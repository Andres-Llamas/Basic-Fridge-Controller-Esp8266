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

  async function getJSON(path) {
    const r = await fetch(path, { cache: 'no-store' });
    setDot(r.ok);
    if (!r.ok) throw new Error(await r.text());
    const ct = r.headers.get('content-type') || '';
    if (ct.includes('application/json')) return r.json();
    return r.text();
  }
  function qs(params) { return Object.entries(params).map(([k,v]) => `${encodeURIComponent(k)}=${encodeURIComponent(v)}`).join('&'); }

  async function refresh() {
    try {
      const s = await getJSON('/sensors');
      $('#boxTemp').textContent = (s.temperature ?? '--') + ' °C';
      $('#compressor').textContent = s.freezeActive ? 'ON' : 'OFF';
      $('#defrost').textContent = s.defrostActive ? 'ON' : 'OFF';
      $('#time').textContent = s.time ?? '--';
    } catch {}

    try {
      const cfg = await getJSON('/getConfig');
      $('#setpoint').textContent = cfg.setpoint ?? '--';
      $('#threshold').textContent = cfg.threshold ?? '--';
      $('#inpSetpoint').value = cfg.setpoint ?? 4;
      $('#inpThreshold').value = cfg.threshold ?? 1;
      renderTimers(cfg.timers || []);
    } catch {}
  }

  function renderTimers(arr) {
    const box = $('#timers');
    box.innerHTML = '';
    if (!arr.length) { box.textContent = 'No timers set'; return; }
    const list = document.createElement('div');
    list.className = 'grid';
    arr.forEach((t,idx) => {
      const d = document.createElement('div');
      d.innerHTML = `<strong>Timer ${idx}</strong><span>${t.start} → ${t.stop}</span>`;
      list.appendChild(d);
    });
    box.appendChild(list);
  }

  // Controls
  $('#refresh').addEventListener('click', refresh);
  $('#btnSetpoint').addEventListener('click', async () => {
    const v = parseFloat($('#inpSetpoint').value);
    await fetch('/setFrTemp?'+qs({temp:v}));
    refresh();
  });
  $('#btnThreshold').addEventListener('click', async () => {
    const v = parseFloat($('#inpThreshold').value);
    await fetch('/setTrhTemp?'+qs({temp:v}));
    refresh();
  });
  $('#btnFreeze').addEventListener('click', async () => { await fetch('/checkFreeze'); refresh(); });
  $('#btnDefrost').addEventListener('click', async () => { await fetch('/checkDefrost'); refresh(); });
  $('#btnSaveTimer').addEventListener('click', async () => {
    const idx = parseInt($('#idx').value,10);
    const [sh, sm] = $('#tStart').value.split(':').map(Number);
    const [eh, em] = $('#tStop').value.split(':').map(Number);
    const url = '/timersDef?' + qs({ indexToSet: idx, startHour: sh, startMinute: sm, stopHour: eh, stopMinute: em });
    const r = await fetch(url);
    if (!r.ok) alert(await r.text());
    refresh();
  });

  // Wi-Fi panel
  async function loadWifi() {
    try {
      const w = await getJSON('/wifiGet');
      $('#wifiSsid').value = w.ssid || '';
    } catch {}
  }
  $('#wifiSave').addEventListener('click', async () => {
    const ssid = $('#wifiSsid').value;
    const pass = $('#wifiPass').value;
    const r = await fetch('/wifiSet?'+qs({ssid, pass}));
    if (r.ok) alert('Saved. Device will reboot and reconnect.');
  });
  $('#wifiScan').addEventListener('click', async () => {
    const list = await getJSON('/wifiScan');
    const box = $('#wifiList'); box.innerHTML = '';
    list.forEach(n => {
      const d = document.createElement('div');
      d.innerHTML = `<strong>${n.ssid}</strong><span>RSSI ${n.rssi} dBm</span>`;
      d.onclick = () => { $('#wifiSsid').value = n.ssid; };
      box.appendChild(d);
    });
  });
  $('#wifiReset').addEventListener('click', async () => {
    if (!confirm('Clear creds and reboot?')) return;
    await fetch('/wifiReset'); alert('Cleared. Rebooting…');
  });

  refresh(); loadWifi(); setInterval(refresh, 5000);
})();