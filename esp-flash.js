import { ESPLoader, Transport } from './vendor/esptool-js/lib/index.js';

const BAUDRATE = 921600;
const FLASH_ADDRESS = 0;
const BIN_FOLDER = 'radio';
const FILES_JSON_KEY = 'transmitter';

const LANGS = {
  zh: {
    title: 'ESP 固件烧录',
    subtitle: 'ESP8285 · 请选择固件后连接设备并烧录',
    hint: '烧录地址固定为 0x0，波特率 921600，Flash 参数保持芯片原样。请使用 Chrome / Edge。',
    'fw-label': '固件文件 (bin/radio)',
    'fw-loading': '正在加载固件列表…',
    'fw-empty': '未找到固件，请确认 bin/radio 目录',
    'flash-btn': '连接并烧录',
    'disconnect-btn': '断开连接',
    'back-link': '返回配置工具',
    'status-ready': '请选择固件',
    'status-connecting': '正在连接设备…',
    'status-loading-fw': '正在加载固件…',
    'status-flashing': '正在烧录…',
    'status-done': '烧录完成，请重新上电或返回配置工具',
    'status-no-serial': '浏览器不支持 Web Serial，请使用 Chrome / Edge',
    'status-no-fw': '请先选择固件文件',
    'status-fetch-fw-fail': '固件文件加载失败',
    'chip-detected': '已识别芯片：',
    'err-prefix': '失败：',
  },
  en: {
    title: 'ESP Firmware Flash',
    subtitle: 'ESP8285 · Select firmware, connect, and flash',
    hint: 'Flash address 0x0, baud 921600, flash params kept. Use Chrome / Edge.',
    'fw-label': 'Firmware (bin/radio)',
    'fw-loading': 'Loading firmware list…',
    'fw-empty': 'No firmware found in bin/radio',
    'flash-btn': 'Connect & Flash',
    'disconnect-btn': 'Disconnect',
    'back-link': 'Back to Config',
    'status-ready': 'Select a firmware file',
    'status-connecting': 'Connecting…',
    'status-loading-fw': 'Loading firmware…',
    'status-flashing': 'Flashing…',
    'status-done': 'Done. Power-cycle or return to config tool.',
    'status-no-serial': 'Web Serial not supported. Use Chrome / Edge.',
    'status-no-fw': 'Select a firmware file first',
    'status-fetch-fw-fail': 'Failed to load firmware file',
    'chip-detected': 'Chip: ',
    'err-prefix': 'Failed: ',
  },
};

let currentLang = 'zh';
let device = null;
let transport = null;
let esploader = null;
let firmwareData = null;
let busy = false;

const $ = (id) => document.getElementById(id);

function t(key) {
  return (LANGS[currentLang] && LANGS[currentLang][key]) || key;
}

function getBasePath() {
  const path = window.location.pathname;
  const slash = path.lastIndexOf('/');
  return slash >= 0 ? path.substring(0, slash + 1) : './';
}

function setStatus(msg, type) {
  const el = $('status');
  el.textContent = msg;
  el.className = type || '';
}

function logLine(msg) {
  const panel = $('log-panel');
  panel.style.display = 'block';
  panel.textContent += msg + '\n';
  panel.scrollTop = panel.scrollHeight;
}

const terminal = {
  clean() { $('log-panel').textContent = ''; },
  writeLine(data) { logLine(data); },
  write(data) { logLine(String(data)); },
};

function applyLang() {
  document.querySelectorAll('[data-i18n]').forEach((el) => {
    const key = el.getAttribute('data-i18n');
    if (LANGS[currentLang][key]) {
      if (key === 'hint') {
        el.innerHTML = currentLang === 'zh'
          ? '烧录地址固定为 <strong>0x0</strong>，波特率 <strong>921600</strong>，Flash 参数保持芯片原样。请使用 Chrome / Edge。'
          : 'Flash address <strong>0x0</strong>, baud <strong>921600</strong>, flash params kept. Use Chrome / Edge.';
      } else {
        el.textContent = LANGS[currentLang][key];
      }
    }
  });
  $('lang-switcher').textContent = currentLang === 'zh' ? '🌐 EN' : '🌐 中文';
  $('back-link').textContent = t('back-link');
}

async function loadFirmwareList() {
  const sel = $('fw-select');
  sel.innerHTML = '';
  const optLoad = document.createElement('option');
  optLoad.value = '';
  optLoad.textContent = t('fw-loading');
  sel.appendChild(optLoad);

  const base = getBasePath();
  let names = [];
  try {
    const res = await fetch(base + 'bin/files.json');
    if (res.ok) {
      const json = await res.json();
      names = json[FILES_JSON_KEY] || [];
    }
  } catch (e) {
    console.warn('files.json', e);
  }

  sel.innerHTML = '';
  if (!names.length) {
    const o = document.createElement('option');
    o.value = '';
    o.textContent = t('fw-empty');
    sel.appendChild(o);
    $('flash-btn').disabled = true;
    setStatus(t('fw-empty'), 'error');
    return;
  }

  const placeholder = document.createElement('option');
  placeholder.value = '';
  placeholder.textContent = '— ' + t('fw-label') + ' —';
  sel.appendChild(placeholder);

  names.forEach((name) => {
    const o = document.createElement('option');
    o.value = name;
    o.textContent = name;
    sel.appendChild(o);
  });

  $('flash-btn').disabled = false;
  setStatus(t('status-ready'));
}

async function fetchFirmware(name) {
  const url = getBasePath() + 'bin/' + BIN_FOLDER + '/' + encodeURIComponent(name);
  const res = await fetch(url);
  if (!res.ok) throw new Error(t('status-fetch-fw-fail') + ' (' + name + ')');
  return new Uint8Array(await res.arrayBuffer());
}

function setProgress(pct) {
  $('progress-wrap').style.display = 'block';
  $('progress-bar').style.width = pct + '%';
  $('progress-text').textContent = Math.round(pct) + '%';
}

async function connectDevice() {
  if (!navigator.serial) throw new Error(t('status-no-serial'));
  device = await navigator.serial.requestPort();
  transport = new Transport(device, true);
  esploader = new ESPLoader({
    transport,
    baudrate: BAUDRATE,
    terminal,
    debugLogging: false,
  });
  const chip = await esploader.main();
  $('chip-info').textContent = t('chip-detected') + chip;
  $('disconnect-btn').style.display = 'block';
  return chip;
}

async function flashFirmware(data) {
  await esploader.writeFlash({
    fileArray: [{ data, address: FLASH_ADDRESS }],
    eraseAll: false,
    compress: true,
    flashMode: 'keep',
    flashFreq: 'keep',
    flashSize: 'keep',
    reportProgress: (_fileIndex, written, total) => {
      setProgress((written / total) * 100);
    },
  });
  await esploader.after('hard_reset');
}

async function disconnect() {
  try {
    if (transport) await transport.disconnect();
  } catch (e) { /* ignore */ }
  device = null;
  transport = null;
  esploader = null;
  $('disconnect-btn').style.display = 'none';
  $('chip-info').textContent = '';
}

$('fw-select').addEventListener('change', () => {
  firmwareData = null;
  if ($('fw-select').value) setStatus(t('status-ready'));
});

$('flash-btn').addEventListener('click', async () => {
  if (busy) return;
  const fwName = $('fw-select').value;
  if (!fwName) {
    setStatus(t('status-no-fw'), 'error');
    return;
  }

  busy = true;
  $('flash-btn').disabled = true;
  terminal.clean();

  try {
    setStatus(t('status-loading-fw'));
    if (!firmwareData) firmwareData = await fetchFirmware(fwName);

    setStatus(t('status-connecting'));
    await connectDevice();

    setStatus(t('status-flashing'));
    setProgress(0);
    await flashFirmware(firmwareData);

    setProgress(100);
    setStatus(t('status-done'), 'ok');
  } catch (e) {
    console.error(e);
    setStatus(t('err-prefix') + (e.message || e), 'error');
    await disconnect();
  } finally {
    busy = false;
    $('flash-btn').disabled = !$('fw-select').value;
  }
});

$('disconnect-btn').addEventListener('click', async () => {
  await disconnect();
  setStatus(t('status-ready'));
});

$('lang-switcher').addEventListener('click', () => {
  currentLang = currentLang === 'zh' ? 'en' : 'zh';
  applyLang();
  if (!$('status').classList.contains('ok') && !$('status').classList.contains('error')) {
    setStatus(t('status-ready'));
  }
});

applyLang();
loadFirmwareList();
