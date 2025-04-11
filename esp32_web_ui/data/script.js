const canvas = document.getElementById("canvas-animation");
const ctx = canvas.getContext("2d");

function resizeCanvas() {
  canvas.width = window.innerWidth;
  canvas.height = window.innerHeight;
}
resizeCanvas();
window.addEventListener('resize', resizeCanvas);

class Particle {
  constructor() {
    this.reset();
  }
  reset() {
    this.x = Math.random() * canvas.width;
    this.y = Math.random() * canvas.height;
    this.vx = (Math.random() - 0.5) * 2.5;
    this.vy = (Math.random() - 0.5) * 2.5;
    this.radius = Math.random() * 3 + 1;
    this.life = Math.random() * 100 + 50;
  }
  update() {
    this.x += this.vx;
    this.y += this.vy;
    this.life -= 1;
    if (this.x < 0 || this.x > canvas.width) this.vx *= -1;
    if (this.y < 0 || this.y > canvas.height) this.vy *= -1;
    if (this.life < 0) this.reset();
  }
  draw() {
    ctx.beginPath();
    ctx.arc(this.x, this.y, this.radius, 0, Math.PI * 2);
    ctx.fillStyle = "rgba(255,255,255,0.7)";
    ctx.fill();
  }
}

const particles = [];
const particleCount = 500;
for (let i = 0; i < particleCount; i++) {
  particles.push(new Particle());
}

function animateParticles() {
  ctx.clearRect(0, 0, canvas.width, canvas.height);
  particles.forEach(p => {
    p.update();
    p.draw();
  });
  requestAnimationFrame(animateParticles);
}
animateParticles();
const ledButtons = document.querySelectorAll(".led-btn");
const colorInput = document.getElementById("colorInput");

ledButtons.forEach(button => {
  button.addEventListener("click", function() {
    const ledIndex = this.getAttribute("data-led");
    fetch(`/toggle?led=${ledIndex}`)
      .then(response => response.text())
      .then(data => {
        console.log(`LED ${ledIndex} toggled:`, data);
        updateStatus();
      })
      .catch(err => console.error("Error toggling LED:", err));
  });
});
let lastRgbSendTime = 0;
let rgbSendTimeout = null;

function sendRgbChange(color) {
  const now = Date.now();
  const timeSinceLastSend = now - lastRgbSendTime;
  if (timeSinceLastSend < 100) {
    clearTimeout(rgbSendTimeout);
    rgbSendTimeout = setTimeout(() => {
      lastRgbSendTime = Date.now();
      fetchRgb(color);
    }, 100 - timeSinceLastSend);
  } else {
    lastRgbSendTime = now;
    fetchRgb(color);
  }
}

function fetchRgb(color) {
  const r = parseInt(color.substring(1, 3), 16);
  const g = parseInt(color.substring(3, 5), 16);
  const b = parseInt(color.substring(5, 7), 16);
  fetch(`/setColor?r=${r}&g=${g}&b=${b}`)
    .then(response => response.text())
    .then(data => {
      console.log("Color set (real-time):", data);
      updateStatus();
    })
    .catch(err => console.error("Error setting color:", err));
}

colorInput.addEventListener("input", function() {
  const color = colorInput.value; // فرمت: #RRGGBB
  colorInput.style.backgroundColor = color;
  sendRgbChange(color);
});

function componentToHex(c) {
  let hex = c.toString(16);
  return hex.length == 1 ? "0" + hex : hex;

function updateStatus() {
  fetch('/status')
    .then(response => response.json())
    .then(data => {
      console.log("Status received:", data);
      
      // به روزرسانی وضعیت LEDها
      data.ledStates.forEach((state, index) => {
        const btn = document.querySelector(`.led-btn[data-led="${index}"]`);
        if (btn) {
          if (state == 1) {
            btn.classList.remove("button-off");
            btn.classList.add("button-on");
          } else {
            btn.classList.remove("button-on");
            btn.classList.add("button-off");
          }
        }
      });
      let rgb = data.rgb;
      const correctedR = 255 - rgb.r;
      const correctedG = 255 - rgb.g;
      const correctedB = 255 - rgb.b;
      const hexColor = "#" + componentToHex(correctedR) + componentToHex(correctedG) + componentToHex(correctedB);
      colorInput.value = hexColor;
      colorInput.style.backgroundColor = hexColor;

      document.getElementById("temperature").innerText = `دمای فعلی: ${data.temperature} °C | رطوبت: ${data.humidity} %`;
    })
    .catch(error => {
      console.error("Error fetching status:", error);
    });
}

updateStatus();
setInterval(updateStatus, 5000);
