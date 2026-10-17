'use strict';
// Positions are in the supplied SVG's 2828 x 1473 coordinate space.
// Ranges are replaced by the engine's parameter metadata on connection.
const definitions = [
  ['Chorus_Rate', 'Chorus rate', 245, 315, 90.5, 0, 10, .25, 5],
  ['Chorus_Depth', 'Chorus depth', 553, 315, 90.5, 0, .05, .001, .018],
  ['Chorus_Feedback', 'Chorus feedback', 861, 315, 90.5, 0, 100, 1, 20],
  ['Chorus_Mix', 'Chorus mix', 1169, 315, 90.5, 0, 100, 1, 25],
  ['Delay_TimeL', 'Delay left division', 1657.5, 315.5, 42, 0, 6, 1, 1],
  ['Delay_TimeR', 'Delay right division', 1967.5, 315.5, 42, 0, 6, 1, 3],
  ['Delay_Feedback', 'Delay feedback', 2276, 315, 90.5, 0, 100, 1, 0],
  ['Delay_Mix', 'Delay mix', 2583, 315, 90.5, 0, 100, 1, 50],
  ['Reverb_Size', 'Reverb size', 245, 812, 90.5, 0, 1, .01, .3],
  ['Reverb_Width', 'Reverb width', 503.5, 812.5, 42, 0, 2, 1, 1],
  ['Reverb_EQMode', 'Reverb EQ mode', 811.5, 812.5, 42, 0, 2, 1, 1],
  ['Reverb_Mix', 'Reverb mix', 1169, 812, 90.5, 0, 100, 1, 25],
  ['Limiter_OnOff', 'Limiter', 1053.5, 1268.5, 60, 0, 1, 1, 0, true],
  ['Reverb_Freeze', 'Freeze', 1803.5, 1138.5, 60, 0, 1, 1, 0, true],
  ['Bypass_OnOff', 'Bypass', 2551.5, 1268.5, 60, 0, 1, 1, 0, true],
];
const backend = window.__JUCE__?.backend;
const previewMode = !backend && new URLSearchParams(location.search).has('preview');
const controls = new Map();
const readout = document.querySelector('#readout');
const connection = document.querySelector('#connection');
let connected = previewMode;
let activeKnob = null;
let readoutTimer;
function updateReadout(c) {
  activeKnob = c;
  const text = c ? `${c.label} · ${display(c)}` : '';
  readout.textContent = backend ? '' : text;
  backend?.emitEvent('readout', text);
}
function releaseReadout(c) {
  if (activeKnob === c && !c.drag && !c.hovered) updateReadout(null);
}
const send = (action, control, value) => backend?.emitEvent('control', { action, id: control.id, value });
const snap = (c, value) => Math.min(c.max, Math.max(c.min, c.min + Math.round((value - c.min) / (c.step || .001)) * (c.step || .001)));
function display(c) {
  if (c.toggle) return c.value ? 'On' : 'Off';
  if (c.id.startsWith('Delay_Time')) return ['1/1', '1/2', '1/3', '1/4', '1/6', '1/8', '1/12'][Math.round(c.value)];
  // Match the actual engine: width .90/.48/.10; damping .01/.45/.65.
  if (c.id === 'Reverb_Width') return ['Wide', 'Middle', 'Narrow'][Math.round(c.value)];
  if (c.id === 'Reverb_EQMode') return ['Cold', 'Neutral', 'Warm'][Math.round(c.value)];
  if (c.id === 'Chorus_Rate') return `${c.value.toFixed(2)} Hz`;
  if (c.id === 'Chorus_Depth') return c.value.toFixed(3);
  if (c.id === 'Reverb_Size') return `${Math.round(c.value * 100)}%`;
  return `${Math.round(c.value)}%`;
}
function render(c) {
  const text = display(c);
  c.element.disabled = !connected;
  c.element.querySelector('.value').textContent = text;
  if (c.id === 'Bypass_OnOff') document.querySelector('#pedal').classList.toggle('bypassed', c.value >= .5);
  if (activeKnob === c) updateReadout(c);
  if (c.toggle) c.element.setAttribute('aria-pressed', String(c.value >= .5));
  else {
    let angle = -135 + 270 * (c.value - c.min) / (c.max - c.min);
    if (c.id === 'Reverb_Width' || c.id === 'Reverb_EQMode') angle = 135 - 45 * c.value;
    c.element.querySelector('.indicator').setAttribute('transform', `rotate(${angle} 100 100)`);
    for (const [key, value] of Object.entries({min: c.min, max: c.max, now: c.value, text}))
      c.element.setAttribute(`aria-value${key}`, String(value));
  }
}
function set(c, value) {
  if (!connected) return;
  c.value = snap(c, value);
  render(c);
  if (!c.toggle) {
    updateReadout(c);
    clearTimeout(readoutTimer);
    readoutTimer = setTimeout(() => releaseReadout(c), 600);
  }
  send('set', c, c.value);
}
function end(c) {
  if (!c.drag) return;
  c.drag = null;
  c.element.classList.remove('dragging');
  send('end', c);
  releaseReadout(c);
}
for (const [id, label, x, y, radius, min, max, step, defaultValue, toggle = false] of definitions) {
  const c = {id, label, min, max, step, defaultValue, value: defaultValue, toggle};
  const el = document.createElement('button');
  c.element = el;
  el.id = id;
  el.className = `control ${toggle ? 'footswitch' : 'knob'}`;
  el.setAttribute('aria-label', label);
  if (!toggle) el.setAttribute('role', 'slider');
  // Small selectors get a larger transparent hit target without changing their ring.
  const hitRadius = Math.max(radius, 60);
  el.style.cssText = `left:${x / 2828 * 100}%;top:${y / 1473 * 100}%;width:${hitRadius * 2 / 2828 * 100}%;height:${hitRadius * 2 / 1473 * 100}%`;
  const r = radius / hitRadius * 95;
  el.innerHTML = `<svg viewBox="0 0 200 200" aria-hidden="true"><defs><clipPath id="ring-${id}"><circle cx="100" cy="100" r="${r + 2}"/></clipPath></defs><circle class="ring" cx="100" cy="100" r="${r}" clip-path="url(#ring-${id})"/>${toggle ? '<circle class="dot" cx="100" cy="100" r="49"/>' : `<line class="indicator" x1="100" y1="100" x2="100" y2="${100 - r * .66}"/>`}</svg><span class="value"></span>`;
  if (toggle) el.addEventListener('click', () => set(c, c.value ? 0 : 1));
  else {
    el.addEventListener('pointerenter', () => { c.hovered = true; updateReadout(c); });
    el.addEventListener('pointerleave', () => { c.hovered = false; releaseReadout(c); });
    el.addEventListener('pointerdown', e => {
      if (e.button !== 0 || !connected) return;
      e.preventDefault(); el.focus(); el.setPointerCapture(e.pointerId);
      c.drag = {y: e.clientY, value: c.value};
      updateReadout(c);
      el.classList.add('dragging'); send('begin', c);
    });
    el.addEventListener('pointermove', e => {
      if (!c.drag) return;
      const delta = (c.drag.y - e.clientY) * (c.max - c.min) / (e.shiftKey ? 2000 : 200);
      c.drag.value = Math.min(c.max, Math.max(c.min, c.drag.value + delta));
      c.drag.y = e.clientY;
      set(c, c.drag.value);
    });
    for (const event of ['pointerup', 'pointercancel', 'lostpointercapture']) el.addEventListener(event, () => end(c));
    el.addEventListener('dblclick', () => set(c, radius > 60 ? 0 : c.defaultValue));
    el.addEventListener('keydown', e => {
      const steps = {ArrowUp: 1, ArrowRight: 1, ArrowDown: -1, ArrowLeft: -1, PageUp: 10, PageDown: -10};
      if (e.key in steps) { e.preventDefault(); set(c, c.value + steps[e.key] * c.step); }
      if (e.key === 'Home' || e.key === 'End') { e.preventDefault(); set(c, e.key === 'Home' ? c.min : c.max); }
    });
    el.addEventListener('wheel', e => {
      if (document.activeElement !== el) return;
      e.preventDefault(); set(c, c.value + (e.deltaY < 0 ? 1 : -1) * c.step);
    }, {passive: false});
  }
  el.addEventListener('blur', () => end(c));
  controls.set(id, c);
  document.querySelector('#controls').append(el);
  render(c);
}
function clearInteraction() {
  controls.forEach(c => { c.hovered = false; end(c); });
  clearTimeout(readoutTimer);
  updateReadout(null);
}
window.addEventListener('blur', clearInteraction);
document.addEventListener('visibilitychange', () => { if (document.hidden) clearInteraction(); });
backend?.addEventListener('parameters', values => {
  connected = true;
  connection.textContent = '';
  for (const item of values) {
    const c = controls.get(item.id);
    if (!c || c.drag) continue;
    Object.assign(c, item);
    render(c);
  }
});
if (previewMode) connection.textContent = 'Design preview · Audio engine disconnected';
else if (!backend) connection.textContent = 'Open the plugin to connect to the audio engine';
fetch('faceplate.svg').then(response => {
  if (!response.ok) throw new Error('Faceplate unavailable');
  return response.text();
}).then(svg => {
  const doc = new DOMParser().parseFromString(svg, 'image/svg+xml');
  // The supplied SVG is artwork only. No scripts or external embeds are imported.
  doc.querySelectorAll('script, foreignObject, image, use').forEach(node => node.remove());
  for (const el of doc.querySelectorAll('*'))
    for (const attr of [...el.attributes])
      if (attr.name.startsWith('on') || attr.name.endsWith('href')) el.removeAttribute(attr.name);
  // The logo outlines contain three fused sun rays. Recolour just those
  // narrow radial strips, preserving the surrounding lettering.
  const ns = 'http://www.w3.org/2000/svg';
  const mask = doc.createElementNS(ns, 'mask');
  mask.id = 'logo-rays';
  mask.setAttribute('maskUnits', 'userSpaceOnUse');
  mask.setAttribute('x', '0'); mask.setAttribute('y', '0');
  mask.setAttribute('width', '2828'); mask.setAttribute('height', '1473');
  for (const angle of [-15, 0, 15]) {
    const line = doc.createElementNS(ns, 'line');
    for (const [key, value] of Object.entries({x1: 1650, y1: 696.1, x2: 2700, y2: 696.1,
      stroke: 'white', 'stroke-width': 5, transform: `rotate(${angle} 2340.3 696.1)`}))
      line.setAttribute(key, value);
    mask.append(line);
  }
  doc.querySelector('defs').append(mask);
  const rays = doc.createElementNS(ns, 'g');
  rays.setAttribute('class', 'bb-logo-rays');
  rays.setAttribute('mask', 'url(#logo-rays)');
  [...doc.querySelector('.bb-text').children].slice(-6).forEach(path => rays.append(path.cloneNode(true)));
  doc.documentElement.append(rays);
  document.querySelector('#art').append(document.importNode(doc.documentElement, true));
}).catch(() => { connection.textContent = 'Unable to load faceplate artwork'; });


