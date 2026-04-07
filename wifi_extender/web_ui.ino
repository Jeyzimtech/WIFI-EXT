// Web UI - Terminal Style Blue Dashboard
// This file is part of wifi_extender.ino

const char PAGE_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>ESP32 WiFi Extender</title>
<style>
*{margin:0;padding:0;box-sizing:border-box}
@import url('https://fonts.googleapis.com/css2?family=JetBrains+Mono:wght@300;400;500;700&family=Share+Tech+Mono&display=swap');
:root{
--bg:#060a1a;--panel:#0a1029;--border:#0f3460;--accent:#00d4ff;
--accent2:#0099cc;--text:#c8d6e5;--text-dim:#5a6a7a;--danger:#ff3860;
--success:#00e676;--warning:#ffc107;--glow:0 0 15px rgba(0,212,255,0.3);
}
body{font-family:'JetBrains Mono',monospace;background:var(--bg);color:var(--text);min-height:100vh;overflow-x:hidden}
body::before{content:'';position:fixed;top:0;left:0;width:100%;height:100%;background:repeating-linear-gradient(0deg,transparent,transparent 2px,rgba(0,212,255,0.015) 2px,rgba(0,212,255,0.015) 4px);pointer-events:none;z-index:9999}

/* Header */
.header{background:linear-gradient(135deg,#060e2e 0%,#0a1845 100%);border-bottom:1px solid var(--border);padding:12px 24px;display:flex;align-items:center;justify-content:space-between;position:sticky;top:0;z-index:100}
.logo{display:flex;align-items:center;gap:12px}
.logo-icon{width:36px;height:36px;border:2px solid var(--accent);border-radius:50%;display:flex;align-items:center;justify-content:center;animation:pulse 2s infinite;font-size:16px}
@keyframes pulse{0%,100%{box-shadow:0 0 5px var(--accent)}50%{box-shadow:0 0 20px var(--accent),0 0 40px rgba(0,212,255,0.2)}}
.logo h1{font-family:'Share Tech Mono',monospace;font-size:16px;color:var(--accent);letter-spacing:3px;text-transform:uppercase;text-shadow:var(--glow)}
.logo .ver{font-size:10px;color:var(--text-dim);letter-spacing:1px}
.header-status{display:flex;gap:16px;align-items:center;font-size:11px}
.status-dot{width:8px;height:8px;border-radius:50%;display:inline-block;margin-right:6px}
.status-dot.online{background:var(--success);box-shadow:0 0 8px var(--success)}
.status-dot.offline{background:var(--danger);box-shadow:0 0 8px var(--danger)}

/* Navigation */
.nav{display:flex;background:#080d25;border-bottom:1px solid var(--border);overflow-x:auto}
.nav-btn{background:none;border:none;color:var(--text-dim);padding:12px 20px;font-family:'JetBrains Mono',monospace;font-size:12px;cursor:pointer;border-bottom:2px solid transparent;transition:all 0.3s;white-space:nowrap;letter-spacing:1px}
.nav-btn:hover{color:var(--accent);background:rgba(0,212,255,0.05)}
.nav-btn.active{color:var(--accent);border-bottom-color:var(--accent);background:rgba(0,212,255,0.08)}
.nav-btn::before{content:'> ';opacity:0;transition:opacity 0.3s}
.nav-btn:hover::before,.nav-btn.active::before{opacity:1}

/* Main */
.main{padding:20px;max-width:1400px;margin:0 auto}
.tab{display:none}.tab.active{display:block}

/* Cards */
.card{background:var(--panel);border:1px solid var(--border);border-radius:6px;margin-bottom:16px;overflow:hidden}
.card-header{padding:12px 16px;border-bottom:1px solid var(--border);display:flex;justify-content:space-between;align-items:center;background:rgba(0,212,255,0.03)}
.card-header h3{font-size:12px;color:var(--accent);letter-spacing:2px;text-transform:uppercase}
.card-body{padding:16px}

/* Grid */
.grid{display:grid;gap:16px}
.grid-2{grid-template-columns:repeat(auto-fit,minmax(280px,1fr))}
.grid-3{grid-template-columns:repeat(auto-fit,minmax(200px,1fr))}
.grid-4{grid-template-columns:repeat(auto-fit,minmax(150px,1fr))}

/* Stat Boxes */
.stat{text-align:center;padding:16px}
.stat-value{font-size:28px;font-weight:700;color:var(--accent);text-shadow:var(--glow)}
.stat-label{font-size:10px;color:var(--text-dim);letter-spacing:2px;margin-top:4px;text-transform:uppercase}
.stat-bar{height:3px;background:var(--border);margin-top:10px;border-radius:2px;overflow:hidden}
.stat-bar-fill{height:100%;background:linear-gradient(90deg,var(--accent),var(--accent2));border-radius:2px;transition:width 1s}

/* Forms */
.form-group{margin-bottom:14px}
.form-label{display:block;font-size:11px;color:var(--accent);letter-spacing:1px;margin-bottom:6px;text-transform:uppercase}
.form-input{width:100%;background:#050a1f;border:1px solid var(--border);color:var(--text);padding:10px 14px;border-radius:4px;font-family:'JetBrains Mono',monospace;font-size:13px;transition:all 0.3s}
.form-input:focus{outline:none;border-color:var(--accent);box-shadow:0 0 10px rgba(0,212,255,0.15)}
select.form-input{cursor:pointer}

/* Buttons */
.btn{padding:10px 20px;border:1px solid var(--border);background:rgba(0,212,255,0.08);color:var(--accent);font-family:'JetBrains Mono',monospace;font-size:12px;border-radius:4px;cursor:pointer;transition:all 0.3s;letter-spacing:1px;text-transform:uppercase}
.btn:hover{background:rgba(0,212,255,0.15);box-shadow:var(--glow);transform:translateY(-1px)}
.btn-primary{background:linear-gradient(135deg,#0066aa,#0099dd);border-color:#0088cc;color:#fff}
.btn-primary:hover{background:linear-gradient(135deg,#0077bb,#00aaee)}
.btn-danger{border-color:var(--danger);color:var(--danger);background:rgba(255,56,96,0.08)}
.btn-danger:hover{background:rgba(255,56,96,0.2)}
.btn-sm{padding:6px 12px;font-size:10px}
.btn-group{display:flex;gap:8px;flex-wrap:wrap;margin-top:12px}

/* Tables */
.table-wrap{overflow-x:auto}
table{width:100%;border-collapse:collapse;font-size:12px}
th{text-align:left;padding:10px 14px;color:var(--accent);font-size:10px;letter-spacing:2px;text-transform:uppercase;border-bottom:1px solid var(--border);background:rgba(0,212,255,0.03)}
td{padding:10px 14px;border-bottom:1px solid rgba(15,52,96,0.4)}
tr:hover td{background:rgba(0,212,255,0.03)}
.badge{display:inline-block;padding:3px 8px;border-radius:3px;font-size:10px;font-weight:500}
.badge-ok{background:rgba(0,230,118,0.15);color:var(--success);border:1px solid rgba(0,230,118,0.3)}
.badge-warn{background:rgba(255,193,7,0.15);color:var(--warning);border:1px solid rgba(255,193,7,0.3)}
.badge-err{background:rgba(255,56,96,0.15);color:var(--danger);border:1px solid rgba(255,56,96,0.3)}

/* Signal */
.signal-bars{display:inline-flex;gap:2px;align-items:flex-end;height:16px}
.signal-bars span{width:4px;background:var(--border);border-radius:1px}
.signal-bars span.active{background:var(--accent)}

/* WiFi List */
.wifi-item{display:flex;justify-content:space-between;align-items:center;padding:12px 16px;border-bottom:1px solid rgba(15,52,96,0.3);cursor:pointer;transition:all 0.2s}
.wifi-item:hover{background:rgba(0,212,255,0.05)}
.wifi-item:last-child{border-bottom:none}
.wifi-name{font-size:13px;color:var(--text)}
.wifi-info{display:flex;align-items:center;gap:12px;font-size:11px;color:var(--text-dim)}

/* Terminal Log */
.terminal{background:#020510;border:1px solid var(--border);border-radius:4px;padding:12px;font-size:11px;max-height:200px;overflow-y:auto;line-height:1.8}
.terminal .t-line{opacity:0;animation:fadeIn 0.3s forwards}
@keyframes fadeIn{to{opacity:1}}
.t-time{color:var(--text-dim)}
.t-ok{color:var(--success)}
.t-err{color:var(--danger)}
.t-info{color:var(--accent)}
.t-warn{color:var(--warning)}

/* Modal */
.modal-overlay{display:none;position:fixed;top:0;left:0;width:100%;height:100%;background:rgba(0,0,0,0.7);z-index:200;align-items:center;justify-content:center}
.modal-overlay.show{display:flex}
.modal{background:var(--panel);border:1px solid var(--accent);border-radius:8px;padding:24px;width:90%;max-width:420px;box-shadow:0 0 30px rgba(0,212,255,0.15)}
.modal h3{color:var(--accent);font-size:14px;margin-bottom:16px;letter-spacing:2px}
.modal-close{float:right;background:none;border:none;color:var(--text-dim);font-size:18px;cursor:pointer}

/* Scrollbar */
::-webkit-scrollbar{width:6px;height:6px}
::-webkit-scrollbar-track{background:var(--bg)}
::-webkit-scrollbar-thumb{background:var(--border);border-radius:3px}
::-webkit-scrollbar-thumb:hover{background:var(--accent)}

/* Loading */
.spinner{width:20px;height:20px;border:2px solid var(--border);border-top-color:var(--accent);border-radius:50%;animation:spin 0.8s linear infinite;display:inline-block}
@keyframes spin{to{transform:rotate(360deg)}}

/* Responsive */
@media(max-width:600px){
.header{flex-direction:column;gap:8px;text-align:center}
.header-status{justify-content:center}
.main{padding:12px}
.grid-2,.grid-3,.grid-4{grid-template-columns:1fr}
.stat-value{font-size:22px}
}
</style>
</head>
<body>

<!-- HEADER -->
<div class="header">
  <div class="logo">
    <div class="logo-icon">📡</div>
    <div>
      <h1>WiFi Extender</h1>
      <div class="ver">ESP32 Control Panel v2.0</div>
    </div>
  </div>
  <div class="header-status">
    <span><span class="status-dot" id="srcStatus"></span>SOURCE: <span id="srcLabel">--</span></span>
    <span><span class="status-dot online"></span>AP: ACTIVE</span>
    <span id="uptimeDisplay">00:00:00</span>
  </div>
</div>

<!-- NAV -->
<div class="nav">
  <button class="nav-btn active" onclick="showTab('dashboard')">Dashboard</button>
  <button class="nav-btn" onclick="showTab('devices')">Devices</button>
  <button class="nav-btn" onclick="showTab('network')">Network</button>
  <button class="nav-btn" onclick="showTab('settings')">Settings</button>
  <button class="nav-btn" onclick="showTab('logs')">System Log</button>
</div>

<!-- MAIN CONTENT -->
<div class="main">

<!-- DASHBOARD TAB -->
<div class="tab active" id="tab-dashboard">
  <div class="grid grid-4">
    <div class="card"><div class="stat"><div class="stat-value" id="sClients">0</div><div class="stat-label">Connected</div><div class="stat-bar"><div class="stat-bar-fill" id="clientBar" style="width:0%"></div></div></div></div>
    <div class="card"><div class="stat"><div class="stat-value" id="sRSSI">--</div><div class="stat-label">Signal dBm</div><div class="stat-bar"><div class="stat-bar-fill" id="rssiBar" style="width:0%"></div></div></div></div>
    <div class="card"><div class="stat"><div class="stat-value" id="sUptime">--</div><div class="stat-label">Uptime</div><div class="stat-bar"><div class="stat-bar-fill" style="width:100%"></div></div></div></div>
    <div class="card"><div class="stat"><div class="stat-value" id="sHeap">--</div><div class="stat-label">Free Memory</div><div class="stat-bar"><div class="stat-bar-fill" id="heapBar" style="width:0%"></div></div></div></div>
  </div>

  <div class="grid grid-2">
    <div class="card">
      <div class="card-header"><h3>📶 Source Network</h3></div>
      <div class="card-body">
        <table>
          <tr><td style="color:var(--text-dim)">SSID</td><td id="dSrcSSID">--</td></tr>
          <tr><td style="color:var(--text-dim)">IP Address</td><td id="dStaIP">--</td></tr>
          <tr><td style="color:var(--text-dim)">Signal</td><td id="dRSSI">--</td></tr>
          <tr><td style="color:var(--text-dim)">Status</td><td id="dSrcStat">--</td></tr>
        </table>
      </div>
    </div>
    <div class="card">
      <div class="card-header"><h3>📡 Access Point</h3></div>
      <div class="card-body">
        <table>
          <tr><td style="color:var(--text-dim)">SSID</td><td id="dApSSID">--</td></tr>
          <tr><td style="color:var(--text-dim)">IP Address</td><td id="dApIP">--</td></tr>
          <tr><td style="color:var(--text-dim)">Clients</td><td id="dClients">--</td></tr>
          <tr><td style="color:var(--text-dim)">Chip</td><td id="dChip">--</td></tr>
        </table>
      </div>
    </div>
  </div>

  <div class="card">
    <div class="card-header"><h3>🖥️ System Terminal</h3></div>
    <div class="card-body"><div class="terminal" id="sysTerminal"></div></div>
  </div>
</div>

<!-- DEVICES TAB -->
<div class="tab" id="tab-devices">
  <div class="card">
    <div class="card-header">
      <h3>👥 Connected Devices</h3>
      <button class="btn btn-sm" onclick="refreshClients()">⟳ Refresh</button>
    </div>
    <div class="card-body">
      <div class="table-wrap">
        <table id="devicesTable">
          <thead><tr><th>Device</th><th>MAC Address</th><th>IP Address</th><th>Connected</th><th>Time Limit</th><th>Status</th><th>Actions</th></tr></thead>
          <tbody id="devicesList"></tbody>
        </table>
      </div>
    </div>
  </div>
</div>

<!-- NETWORK TAB -->
<div class="tab" id="tab-network">
  <div class="card">
    <div class="card-header">
      <h3>🔍 Available Networks</h3>
      <button class="btn btn-sm" onclick="scanNetworks()">⟳ Scan</button>
    </div>
    <div class="card-body" id="networkList"><p style="color:var(--text-dim)">Click Scan to find networks...</p></div>
  </div>
</div>

<!-- SETTINGS TAB -->
<div class="tab" id="tab-settings">
  <div class="grid grid-2">
    <div class="card">
      <div class="card-header"><h3>📡 AP Settings</h3></div>
      <div class="card-body">
        <div class="form-group"><label class="form-label">Network Name (SSID)</label><input class="form-input" id="setApSSID" placeholder="ESP32_Extender"></div>
        <div class="form-group"><label class="form-label">Password</label><input class="form-input" id="setApPass" type="password" placeholder="Min 8 characters"></div>
        <div class="btn-group"><button class="btn btn-primary" onclick="saveAPSettings()">💾 Save AP Settings</button></div>
      </div>
    </div>
    <div class="card">
      <div class="card-header"><h3>🔐 Admin Panel</h3></div>
      <div class="card-body">
        <div class="form-group"><label class="form-label">Admin Username</label><input class="form-input" id="setAdmUser" placeholder="admin"></div>
        <div class="form-group"><label class="form-label">Admin Password</label><input class="form-input" id="setAdmPass" type="password" placeholder="New password"></div>
        <div class="btn-group"><button class="btn btn-primary" onclick="saveAdminSettings()">💾 Save Admin</button></div>
      </div>
    </div>
  </div>
  <div class="card">
    <div class="card-header"><h3>⚙️ System</h3></div>
    <div class="card-body">
      <div class="btn-group">
        <button class="btn btn-danger" onclick="if(confirm('Reboot device?'))rebootDevice()">🔄 Reboot Device</button>
        <button class="btn btn-danger" onclick="if(confirm('Factory reset?'))factoryReset()">⚠️ Factory Reset</button>
      </div>
    </div>
  </div>
</div>

<!-- LOGS TAB -->
<div class="tab" id="tab-logs">
  <div class="card">
    <div class="card-header"><h3>📋 System Log</h3><button class="btn btn-sm" onclick="clearLog()">Clear</button></div>
    <div class="card-body"><div class="terminal" id="logTerminal" style="max-height:500px"></div></div>
  </div>
</div>

</div>

<!-- TIME LIMIT MODAL -->
<div class="modal-overlay" id="timeLimitModal">
  <div class="modal">
    <button class="modal-close" onclick="closeModal()">&times;</button>
    <h3>⏱ SET TIME LIMIT</h3>
    <p style="font-size:11px;color:var(--text-dim);margin-bottom:16px">Device: <span id="tlDevice"></span></p>
    <div class="form-group">
      <label class="form-label">Duration (minutes)</label>
      <select class="form-input" id="tlMinutes">
        <option value="0">Unlimited</option>
        <option value="15">15 Minutes</option>
        <option value="30">30 Minutes</option>
        <option value="60">1 Hour</option>
        <option value="120">2 Hours</option>
        <option value="480">8 Hours</option>
        <option value="1440">24 Hours</option>
      </select>
    </div>
    <div class="btn-group"><button class="btn btn-primary" onclick="applyTimeLimit()">Apply</button><button class="btn" onclick="closeModal()">Cancel</button></div>
  </div>
</div>

<script>
let currentTLMac='';

function showTab(name){
  document.querySelectorAll('.tab').forEach(t=>t.classList.remove('active'));
  document.querySelectorAll('.nav-btn').forEach(b=>b.classList.remove('active'));
  document.getElementById('tab-'+name).classList.add('active');
  event.target.classList.add('active');
  if(name==='devices')refreshClients();
}

function log(msg,type='info'){
  const t=document.getElementById('sysTerminal');
  const l=document.getElementById('logTerminal');
  const now=new Date().toLocaleTimeString();
  const line=`<div class="t-line"><span class="t-time">[${now}]</span> <span class="t-${type}">${msg}</span></div>`;
  t.innerHTML+=line;l.innerHTML+=line;
  t.scrollTop=t.scrollHeight;l.scrollTop=l.scrollHeight;
}

async function fetchStatus(){
  try{
    const r=await fetch('/api/status');
    const d=await r.json();
    document.getElementById('sClients').textContent=d.clientCount;
    document.getElementById('sRSSI').textContent=d.rssi||'--';
    document.getElementById('clientBar').style.width=(d.clientCount/10*100)+'%';
    const rssiP=Math.min(100,Math.max(0,(d.rssi+100)*2));
    document.getElementById('rssiBar').style.width=rssiP+'%';
    const h=Math.floor(d.uptime/3600),m=Math.floor((d.uptime%3600)/60),s=d.uptime%60;
    const ut=`${String(h).padStart(2,'0')}:${String(m).padStart(2,'0')}:${String(s).padStart(2,'0')}`;
    document.getElementById('sUptime').textContent=ut;
    document.getElementById('uptimeDisplay').textContent=ut;
    const heapKB=Math.round(d.freeHeap/1024);
    document.getElementById('sHeap').textContent=heapKB+'K';
    document.getElementById('heapBar').style.width=Math.min(100,(d.freeHeap/320000)*100)+'%';
    document.getElementById('dSrcSSID').textContent=d.sourceSSID||'Not Set';
    document.getElementById('dStaIP').textContent=d.staIP;
    document.getElementById('dRSSI').textContent=d.rssi?d.rssi+' dBm':'N/A';
    document.getElementById('dSrcStat').innerHTML=d.connectedToSource?'<span class="badge badge-ok">Connected</span>':'<span class="badge badge-err">Disconnected</span>';
    document.getElementById('dApSSID').textContent=d.apSSID;
    document.getElementById('dApIP').textContent=d.apIP;
    document.getElementById('dClients').textContent=d.clientCount+' / 10';
    document.getElementById('dChip').textContent=d.chipModel;
    const dot=document.getElementById('srcStatus');
    dot.className='status-dot '+(d.connectedToSource?'online':'offline');
    document.getElementById('srcLabel').textContent=d.connectedToSource?d.sourceSSID:'DISCONNECTED';
  }catch(e){log('Status fetch failed','err');}
}

async function refreshClients(){
  try{
    const r=await fetch('/api/clients');
    const d=await r.json();
    const tb=document.getElementById('devicesList');
    if(!d.clients.length){tb.innerHTML='<tr><td colspan="7" style="text-align:center;color:var(--text-dim)">No devices connected</td></tr>';return;}
    tb.innerHTML=d.clients.map((c,i)=>{
      const mins=Math.floor(c.connected/60),secs=c.connected%60;
      const ct=`${mins}m ${secs}s`;
      const tl=c.timeLimit?c.timeLimit+'min':'∞';
      const st=c.blocked?'<span class="badge badge-err">Blocked</span>':'<span class="badge badge-ok">Active</span>';
      return `<tr>
        <td>${c.hostname}</td><td style="font-size:11px">${c.mac}</td><td>${c.ip}</td>
        <td>${ct}</td><td>${tl}</td><td>${st}</td>
        <td><button class="btn btn-sm" onclick="openTimeLimit('${c.mac}','${c.hostname}')">⏱</button>
        <button class="btn btn-sm ${c.blocked?'btn-primary':'btn-danger'}" onclick="toggleBlock('${c.mac}',${!c.blocked})">${c.blocked?'Unblock':'Block'}</button></td>
      </tr>`;
    }).join('');
  }catch(e){log('Client fetch failed','err');}
}

async function scanNetworks(){
  const el=document.getElementById('networkList');
  el.innerHTML='<div style="text-align:center;padding:20px"><div class="spinner"></div><p style="margin-top:10px;color:var(--text-dim);font-size:12px">Scanning...</p></div>';
  log('Scanning for networks...');
  try{
    const r=await fetch('/api/scan');
    const d=await r.json();
    if(!d.networks.length){el.innerHTML='<p style="color:var(--text-dim)">No networks found</p>';return;}
    el.innerHTML=d.networks.map(n=>{
      const bars=getSignalBars(n.rssi);
      return `<div class="wifi-item" onclick="connectToNetwork('${n.ssid}')">
        <div><div class="wifi-name">${n.ssid}</div><div style="font-size:10px;color:var(--text-dim)">Ch ${n.channel} | ${n.encryption}</div></div>
        <div class="wifi-info">${n.rssi} dBm ${bars}</div></div>`;
    }).join('');
    log(`Found ${d.networks.length} networks`,'ok');
  }catch(e){el.innerHTML='<p style="color:var(--danger)">Scan failed</p>';log('Scan error','err');}
}

function getSignalBars(rssi){
  const level=rssi>-50?4:rssi>-60?3:rssi>-70?2:1;
  let bars='<div class="signal-bars">';
  for(let i=1;i<=4;i++)bars+=`<span style="height:${i*4}px" class="${i<=level?'active':''}"></span>`;
  return bars+'</div>';
}

function connectToNetwork(ssid){
  const pw=prompt(`Enter password for "${ssid}":`);
  if(pw===null)return;
  log(`Connecting to ${ssid}...`);
  fetch('/api/connect',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({ssid:ssid,password:pw})})
  .then(()=>{log('Connection request sent. Please wait...','warn');setTimeout(fetchStatus,8000);})
  .catch(()=>log('Connection failed','err'));
}

function saveAPSettings(){
  const ssid=document.getElementById('setApSSID').value;
  const pass=document.getElementById('setApPass').value;
  if(pass&&pass.length<8){alert('Password must be at least 8 characters');return;}
  const data={};
  if(ssid)data.apSSID=ssid;
  if(pass)data.apPassword=pass;
  fetch('/api/settings',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(data)})
  .then(r=>r.json()).then(d=>{log('AP settings saved: '+d.message,'ok');alert(d.message);})
  .catch(()=>log('Save failed','err'));
}

function saveAdminSettings(){
  const user=document.getElementById('setAdmUser').value;
  const pass=document.getElementById('setAdmPass').value;
  const data={};
  if(user)data.adminUser=user;
  if(pass)data.adminPass=pass;
  fetch('/api/settings',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(data)})
  .then(r=>r.json()).then(d=>{log('Admin settings saved','ok');alert(d.message);})
  .catch(()=>log('Save failed','err'));
}

function toggleBlock(mac,block){
  fetch('/api/block',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({mac:mac,block:block})})
  .then(()=>{log(`Device ${mac} ${block?'blocked':'unblocked'}`,block?'warn':'ok');refreshClients();})
  .catch(()=>log('Action failed','err'));
}

function openTimeLimit(mac,name){
  currentTLMac=mac;
  document.getElementById('tlDevice').textContent=name+' ('+mac+')';
  document.getElementById('timeLimitModal').classList.add('show');
}
function closeModal(){document.getElementById('timeLimitModal').classList.remove('show');}
function applyTimeLimit(){
  const mins=parseInt(document.getElementById('tlMinutes').value);
  fetch('/api/timelimit',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({mac:currentTLMac,minutes:mins})})
  .then(()=>{log(`Time limit set: ${mins?mins+' min':'Unlimited'} for ${currentTLMac}`,'ok');closeModal();refreshClients();})
  .catch(()=>log('Failed','err'));
}

function rebootDevice(){fetch('/api/reboot').then(()=>log('Rebooting...','warn'));}
function factoryReset(){log('Factory reset not yet implemented','warn');}
function clearLog(){document.getElementById('logTerminal').innerHTML='';log('Log cleared');}

// Boot sequence
log('System initializing...','info');
setTimeout(()=>log('Web interface loaded','ok'),500);
setTimeout(()=>log('Fetching system status...','info'),1000);
setTimeout(fetchStatus,1500);
setInterval(fetchStatus,5000);
</script>
</body>
</html>
)rawliteral";

void handleRoot() {
  server.send(200, "text/html", PAGE_HTML);
}

void handleNotFound() {
  server.send(200, "text/html", PAGE_HTML);
}
