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
  button { font-size: 1.1rem; padding: 12px 18px; margin: 8px 8px 0 0; border: 0; border-radius: 10px; cursor: pointer; }
  #say { background:#2563eb; color:#fff; } #stop { background:#6b7280; color:#fff; }
  #status { margin-top: 12px; min-height: 1.4em; color:#16a34a; }
</style>
</head>
<body>
  <h1>🤖 EMO — Gõ chữ, robot đọc</h1>
  <textarea id="txt" rows="4" placeholder="Nhập câu tiếng Việt rồi bấm Đọc..."></textarea>
  <div>
    <button id="say" onclick="say()">🔊 Đọc</button>
    <button id="stop" onclick="stop()">⏹ Dừng</button>
  </div>
  <p id="status"></p>
<script>
  const $ = id => document.getElementById(id);
  async function say() {
    const t = $('txt').value.trim();
    if (!t) return;
    $('status').textContent = 'Đang gửi...';
    try { $('status').textContent = await (await fetch('/say?text=' + encodeURIComponent(t))).text(); }
    catch (e) { $('status').textContent = 'Lỗi kết nối robot'; }
  }
  async function stop() { await fetch('/stop'); $('status').textContent = 'Đã dừng'; }
  $('txt').addEventListener('keydown', e => { if (e.ctrlKey && e.key === 'Enter') say(); });
</script>
</body>
</html>
)HTML";
