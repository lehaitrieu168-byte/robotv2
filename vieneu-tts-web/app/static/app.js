// Logic frontend cho VieNeu-TTS Web: chuyển tab, nạp giọng, gọi API, phát audio.
"use strict";

const $ = (sel) => document.querySelector(sel);

const statusEl = $("#status");
const resultEl = $("#result");
const player = $("#player");
const downloadEl = $("#download");
let lastUrl = null;

// ---- Trạng thái / thông báo ----
function showStatus(msg, kind) {
  statusEl.hidden = false;
  statusEl.textContent = msg;
  statusEl.className = "status" + (kind ? " " + kind : "");
}
function clearStatus() {
  statusEl.hidden = true;
  statusEl.textContent = "";
}

// ---- Chuyển tab ----
document.querySelectorAll(".tab").forEach((tab) => {
  tab.addEventListener("click", () => {
    document.querySelectorAll(".tab").forEach((t) => t.classList.remove("active"));
    document.querySelectorAll(".panel").forEach((p) => p.classList.remove("active"));
    tab.classList.add("active");
    $("#panel-" + tab.dataset.tab).classList.add("active");
  });
});

// ---- Đếm ký tự ----
const presetText = $("#preset-text");
const presetCount = $("#preset-count");
function updateCount() {
  presetCount.textContent = presetText.value.length;
}
presetText.addEventListener("input", updateCount);
updateCount();

// ---- Chèn cue biểu cảm vào con trỏ ----
document.querySelectorAll(".cue").forEach((btn) => {
  btn.addEventListener("click", () => {
    const cue = btn.dataset.cue;
    const start = presetText.selectionStart;
    const end = presetText.selectionEnd;
    const v = presetText.value;
    presetText.value = v.slice(0, start) + " " + cue + " " + v.slice(end);
    presetText.focus();
    updateCount();
  });
});

// ---- Nạp danh sách giọng dựng sẵn ----
async function loadVoices() {
  const sel = $("#preset-voice");
  try {
    const res = await fetch("/api/voices");
    if (!res.ok) throw new Error((await res.json()).detail || res.statusText);
    const data = await res.json();
    sel.innerHTML = "";
    data.voices.forEach((v) => {
      const opt = document.createElement("option");
      opt.value = v.id;
      opt.textContent = v.label;
      sel.appendChild(opt);
    });
    if (!data.voices.length) sel.innerHTML = "<option>Không có giọng</option>";
  } catch (err) {
    sel.innerHTML = "<option>Lỗi tải giọng</option>";
    showStatus("Không nạp được danh sách giọng: " + err.message, "error");
  }
}

// ---- Hiển thị audio kết quả ----
function renderAudio(blob) {
  if (lastUrl) URL.revokeObjectURL(lastUrl);
  lastUrl = URL.createObjectURL(blob);
  player.src = lastUrl;
  downloadEl.href = lastUrl;
  resultEl.hidden = false;
  player.play().catch(() => {});
}

// ---- Gọi API sinh giọng chung ----
async function postAudio(url, body, btn) {
  btn.disabled = true;
  showStatus("Đang tổng hợp giọng nói… (lần đầu có thể tải model, hơi lâu)", "loading");
  try {
    const res = await fetch(url, { method: "POST", body });
    if (!res.ok) {
      let detail = res.statusText;
      try { detail = (await res.json()).detail || detail; } catch (_) {}
      throw new Error(detail);
    }
    renderAudio(await res.blob());
    clearStatus();
  } catch (err) {
    showStatus("Lỗi: " + err.message, "error");
  } finally {
    btn.disabled = false;
  }
}

// ---- Nút: giọng dựng sẵn ----
$("#preset-go").addEventListener("click", () => {
  const text = presetText.value.trim();
  if (!text) return showStatus("Hãy nhập văn bản.", "error");
  const fd = new FormData();
  fd.append("text", text);
  fd.append("voice", $("#preset-voice").value);
  fd.append("style", $("#preset-style").value);
  postAudio("/api/tts", fd, $("#preset-go"));
});

// ---- Nút: nhân bản giọng ----
$("#clone-go").addEventListener("click", () => {
  const text = $("#clone-text").value.trim();
  const file = $("#clone-ref").files[0];
  if (!text) return showStatus("Hãy nhập văn bản.", "error");
  if (!file) return showStatus("Hãy chọn file audio mẫu.", "error");
  const fd = new FormData();
  fd.append("text", text);
  fd.append("ref_audio", file);
  fd.append("denoise", $("#clone-denoise").checked ? "true" : "false");
  postAudio("/api/clone", fd, $("#clone-go"));
});

// ---- Khởi động ----
loadVoices();
