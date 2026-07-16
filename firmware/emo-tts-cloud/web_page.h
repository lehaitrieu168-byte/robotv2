#pragma once
// Trang web nhỏ phục vụ ngay trên ESP32: mở bằng điện thoại/PC, gõ chữ -> robot đọc.
// Để riêng file cho gọn (main .ino dễ đọc). UTF-8 để hiển thị đúng tiếng Việt.

const char INDEX_HTML[] PROGMEM = R"HTML(
<!doctype html>
<html lang="vi">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>EMO nói</title>
<style>
  :root { color-scheme: light dark; }
  body { font-family: system-ui, sans-serif; max-width: 560px; margin: 24px auto; padding: 0 16px; }
  h1 { font-size: 1.3rem; }
  textarea { width: 100%; box-sizing: border-box; font-size: 1.1rem; padding: 10px; border-radius: 10px; }
  select { width: 100%; box-sizing: border-box; font-size: 1rem; padding: 10px; border-radius: 10px; margin-top: 8px; }
  label { display:block; margin-top: 10px; font-size: .9rem; opacity: .8; }
  button { font-size: 1.1rem; padding: 12px 18px; margin: 8px 8px 0 0; border: 0; border-radius: 10px; cursor: pointer; }
  #say { background:#2563eb; color:#fff; } #stop { background:#6b7280; color:#fff; }
  #status { margin-top: 12px; min-height: 1.4em; color:#16a34a; }
</style>
</head>
<body>
  <h1>🤖 EMO — Gõ chữ, robot đọc</h1>
  <textarea id="txt" rows="4" placeholder="Nhập câu tiếng Việt rồi bấm Đọc..."></textarea>
  <label for="voice">Giọng đọc</label>
  <select id="voice"><option value="">Giọng mặc định</option></select>
  <div>
    <button id="say" onclick="say()">🔊 Đọc</button>
    <button id="stop" onclick="stop()">⏹ Dừng</button>
  </div>
  <p id="status"></p>
<script>
  const $ = id => document.getElementById(id);
  async function loadVoices() {
    try {
      const d = await (await fetch('/voices')).json();
      (d.voices || []).forEach(v => {
        const o = document.createElement('option');
        o.value = v.id; o.textContent = v.label;
        $('voice').appendChild(o);
      });
    } catch (e) { /* không có server VieNeu -> chỉ có giọng mặc định */ }
  }
  async function say() {
    const t = $('txt').value.trim();
    if (!t) return;
    $('status').textContent = 'Đang gửi...';
    const q = '/say?text=' + encodeURIComponent(t) + '&voice=' + encodeURIComponent($('voice').value);
    try { $('status').textContent = await (await fetch(q)).text(); }
    catch (e) { $('status').textContent = 'Lỗi kết nối robot'; }
  }
  async function stop() { await fetch('/stop'); $('status').textContent = 'Đã dừng'; }
  $('txt').addEventListener('keydown', e => { if (e.ctrlKey && e.key === 'Enter') say(); });
  loadVoices();
</script>
</body>
</html>
)HTML";
