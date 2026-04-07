// ==========================================
// ESP32 WiFi Extender - Live Dashboard Logic
// ==========================================

const DEFAULT_API_BASE = "http://192.168.4.1";
const isServedByDevice =
  location.protocol.startsWith("http") &&
  (location.hostname === "192.168.4.1" || location.hostname.endsWith(".local"));

let apiBase = isServedByDevice ? "" : (localStorage.getItem("esp32_api_base") || DEFAULT_API_BASE);
let currentTLMac = "";

function buildApiUrl(path) {
  return apiBase ? `${apiBase}${path}` : path;
}

function showTab(name, btn) {
  document.querySelectorAll(".tab").forEach(t => t.classList.remove("active"));
  document.querySelectorAll(".nav-btn").forEach(b => b.classList.remove("active"));
  document.getElementById("tab-" + name).classList.add("active");

  if (btn) {
    btn.classList.add("active");
  } else {
    const targetBtn = Array.from(document.querySelectorAll(".nav-btn")).find(
      b => b.textContent.trim().toLowerCase().includes(name)
    );
    if (targetBtn) targetBtn.classList.add("active");
  }

  if (name === "devices") refreshClients();
}

function log(msg, type = "info") {
  const terminals = ["sysTerminal", "logTerminal"];
  const now = new Date().toLocaleTimeString();
  const line = `<div class="t-line"><span class="t-time">[${now}]</span> <span class="t-${type}">${msg}</span></div>`;

  terminals.forEach(id => {
    const el = document.getElementById(id);
    if (!el) return;
    el.innerHTML += line;
    el.scrollTop = el.scrollHeight;
  });
}

function formatUptime(sec) {
  const h = Math.floor(sec / 3600);
  const m = Math.floor((sec % 3600) / 60);
  const s = sec % 60;
  return `${String(h).padStart(2, "0")}:${String(m).padStart(2, "0")}:${String(s).padStart(2, "0")}`;
}

function formatConnected(sec) {
  const mins = Math.floor(sec / 60);
  const secs = sec % 60;
  return `${mins}m ${secs}s`;
}

function getSignalBars(rssi) {
  const level = rssi > -50 ? 4 : rssi > -60 ? 3 : rssi > -70 ? 2 : 1;
  let bars = '<div class="signal-bars">';
  for (let i = 1; i <= 4; i++) {
    bars += `<span style="height:${i * 4}px" class="${i <= level ? "active" : ""}"></span>`;
  }
  return bars + "</div>";
}

async function fetchStatus() {
  try {
    const r = await fetch(buildApiUrl("/api/status"));
    if (!r.ok) throw new Error(`HTTP ${r.status}`);
    const d = await r.json();

    document.getElementById("sClients").textContent = d.clientCount;
    document.getElementById("sRSSI").textContent = d.rssi ?? "--";
    document.getElementById("sUptime").textContent = formatUptime(d.uptime || 0);
    document.getElementById("uptimeDisplay").textContent = formatUptime(d.uptime || 0);

    const heapKB = Math.round((d.freeHeap || 0) / 1024);
    document.getElementById("sHeap").textContent = `${heapKB}K`;

    document.getElementById("clientBar").style.width = `${Math.min(100, ((d.clientCount || 0) / 10) * 100)}%`;
    const rssiP = Math.min(100, Math.max(0, ((d.rssi || -100) + 100) * 2));
    document.getElementById("rssiBar").style.width = `${rssiP}%`;
    document.getElementById("heapBar").style.width = `${Math.min(100, ((d.freeHeap || 0) / 320000) * 100)}%`;

    document.getElementById("dSrcSSID").textContent = d.sourceSSID || "Not set";
    document.getElementById("dStaIP").textContent = d.staIP || "N/A";
    document.getElementById("dRSSI").textContent = d.connectedToSource ? `${d.rssi} dBm` : "N/A";
    document.getElementById("dSrcStat").innerHTML = d.connectedToSource
      ? '<span class="badge badge-ok">Connected</span>'
      : '<span class="badge badge-err">Disconnected</span>';

    document.getElementById("dApSSID").textContent = d.apSSID || "--";
    document.getElementById("dApIP").textContent = d.apIP || "--";
    document.getElementById("dClients").textContent = `${d.clientCount || 0} / 10`;
    document.getElementById("dChip").textContent = d.chipModel || "--";

    const srcDot = document.getElementById("srcStatus");
    srcDot.className = `status-dot ${d.connectedToSource ? "online" : "offline"}`;
    document.getElementById("srcLabel").textContent = d.connectedToSource ? d.sourceSSID : "DISCONNECTED";
  } catch (e) {
    log(`Status fetch failed from ${buildApiUrl("/api/status")}`, "err");
  }
}

async function refreshClients() {
  try {
    const r = await fetch(buildApiUrl("/api/clients"));
    if (!r.ok) throw new Error(`HTTP ${r.status}`);
    const d = await r.json();
    const tb = document.getElementById("devicesList");

    if (!d.clients || !d.clients.length) {
      tb.innerHTML = '<tr><td colspan="7" style="text-align:center;color:var(--text-dim)">No devices connected</td></tr>';
      return;
    }

    tb.innerHTML = d.clients.map(c => {
      const tl = c.timeLimit ? `${c.timeLimit} min` : "∞ Unlimited";
      const tlBadge = c.timeLimit
        ? `<span class="badge badge-warn">${tl}</span>`
        : `<span style="color:var(--text-dim)">${tl}</span>`;
      const st = c.blocked
        ? '<span class="badge badge-err">Blocked</span>'
        : '<span class="badge badge-ok">Active</span>';

      return `<tr>
        <td><strong style="color:var(--accent)">${c.hostname}</strong></td>
        <td style="font-size:11px;color:var(--text-dim)">${c.mac}</td>
        <td>${c.ip}</td>
        <td>${formatConnected(c.connected)}</td>
        <td>${tlBadge}</td>
        <td>${st}</td>
        <td>
          <button class="btn btn-sm" onclick="openTimeLimit('${c.mac}','${c.hostname}')">⏱</button>
          <button class="btn btn-sm ${c.blocked ? "btn-primary" : "btn-danger"}" onclick="toggleBlock('${c.mac}',${!c.blocked})">${c.blocked ? "Unblock" : "Block"}</button>
        </td>
      </tr>`;
    }).join("");
  } catch (e) {
    log("Client fetch failed", "err");
  }
}

async function scanNetworks() {
  const el = document.getElementById("networkList");
  el.innerHTML = '<div style="text-align:center;padding:20px"><div class="spinner"></div><p style="margin-top:10px;color:var(--text-dim);font-size:12px">Scanning...</p></div>';
  log("Scanning for networks...");

  try {
    const r = await fetch(buildApiUrl("/api/scan"));
    if (!r.ok) throw new Error(`HTTP ${r.status}`);
    const d = await r.json();

    if (!d.networks || !d.networks.length) {
      el.innerHTML = '<p class="dim">No networks found</p>';
      return;
    }

    el.innerHTML = d.networks.map(n => {
      const bars = getSignalBars(n.rssi);
      const lock = n.encryption !== "Open" ? "🔒" : "🔓";
      return `<div class="wifi-item" onclick="connectToNetwork('${n.ssid.replace(/'/g, "\\'")}')">
        <div>
          <div class="wifi-name">${lock} ${n.ssid}</div>
          <div class="wifi-sub">Channel ${n.channel} · ${n.encryption}</div>
        </div>
        <div class="wifi-info"><span>${n.rssi} dBm</span>${bars}</div>
      </div>`;
    }).join("");

    log(`Scan complete: ${d.networks.length} networks found`, "ok");
  } catch (e) {
    el.innerHTML = '<p style="color:var(--danger)">Scan failed. Check API base and ESP32 connection.</p>';
    log("Scan failed", "err");
  }
}

async function connectToNetwork(ssid) {
  const pw = prompt(`Enter password for \"${ssid}\":`);
  if (pw === null) return;

  log(`Connecting to ${ssid}...`, "info");
  try {
    await fetch(buildApiUrl("/api/connect"), {
      method: "POST",
      body: JSON.stringify({ ssid: ssid, password: pw })
    });
    log("Connection request sent. Waiting for result...", "warn");
    setTimeout(fetchStatus, 8000);
  } catch (e) {
    log("Connection request failed", "err");
  }
}

async function saveAPSettings() {
  const ssid = document.getElementById("setApSSID").value.trim();
  const pass = document.getElementById("setApPass").value;
  if (pass && pass.length < 8) {
    alert("Password must be at least 8 characters");
    return;
  }

  const payload = {};
  if (ssid) payload.apSSID = ssid;
  if (pass) payload.apPassword = pass;

  try {
    const r = await fetch(buildApiUrl("/api/settings"), {
      method: "POST",
      body: JSON.stringify(payload)
    });
    const d = await r.json();
    log(`AP settings saved: ${d.message}`, "ok");
    alert(d.message);
  } catch (e) {
    log("Failed to save AP settings", "err");
  }
}

async function saveAdminSettings() {
  const user = document.getElementById("setAdmUser").value.trim();
  const pass = document.getElementById("setAdmPass").value;

  const payload = {};
  if (user) payload.adminUser = user;
  if (pass) payload.adminPass = pass;

  try {
    const r = await fetch(buildApiUrl("/api/settings"), {
      method: "POST",
      body: JSON.stringify(payload)
    });
    const d = await r.json();
    log("Admin settings saved", "ok");
    alert(d.message);
  } catch (e) {
    log("Failed to save admin settings", "err");
  }
}

async function toggleBlock(mac, block) {
  try {
    await fetch(buildApiUrl("/api/block"), {
      method: "POST",
      body: JSON.stringify({ mac: mac, block: block })
    });
    log(`Device ${mac} ${block ? "blocked" : "unblocked"}`, block ? "warn" : "ok");
    refreshClients();
  } catch (e) {
    log("Block/unblock failed", "err");
  }
}

function openTimeLimit(mac, name) {
  currentTLMac = mac;
  document.getElementById("tlDevice").textContent = `${name} (${mac})`;
  document.getElementById("timeLimitModal").classList.add("show");
}

function closeModal() {
  document.getElementById("timeLimitModal").classList.remove("show");
}

async function applyTimeLimit() {
  const mins = parseInt(document.getElementById("tlMinutes").value, 10);
  try {
    await fetch(buildApiUrl("/api/timelimit"), {
      method: "POST",
      body: JSON.stringify({ mac: currentTLMac, minutes: mins })
    });
    log(`Time limit set for ${currentTLMac}: ${mins ? mins + " min" : "Unlimited"}`, "ok");
    closeModal();
    refreshClients();
  } catch (e) {
    log("Failed to set time limit", "err");
  }
}

function rebootDevice() {
  fetch(buildApiUrl("/api/reboot"))
    .then(() => log("Reboot command sent", "warn"))
    .catch(() => log("Reboot request failed", "err"));
}

function factoryReset() {
  log("Factory reset not implemented in firmware yet", "warn");
}

function clearLog() {
  document.getElementById("logTerminal").innerHTML = "";
  log("Log cleared", "info");
}

function configureApiBase() {
  if (isServedByDevice) {
    log("Using same-host API because page is served by ESP32", "ok");
    return;
  }

  const entered = prompt("Enter ESP32 API base URL", apiBase || DEFAULT_API_BASE);
  if (!entered) return;

  apiBase = entered.replace(/\/$/, "");
  localStorage.setItem("esp32_api_base", apiBase);
  log(`API base set to ${apiBase}`, "ok");
  fetchStatus();
}

document.addEventListener("DOMContentLoaded", () => {
  log("Live mode enabled (no dummy data)", "ok");
  if (isServedByDevice) {
    log("API base: same host", "info");
  } else {
    log(`API base: ${apiBase} (use configureApiBase() in console to change)`, "info");
  }

  fetchStatus();
  refreshClients();
  setInterval(fetchStatus, 5000);
});
