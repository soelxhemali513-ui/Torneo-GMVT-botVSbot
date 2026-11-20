(function () {
  'use strict';

  const IS_INDEX   = window.location.pathname.endsWith('index.html') ||
                     window.location.pathname === '/' ||
                     window.location.pathname.endsWith('/');
  const INTRO_DONE = sessionStorage.getItem('gmvt_intro_done') === '1';

  const style = document.createElement('style');
  style.textContent = `
    body.gmvt-loading > *:not(#gmvt-overlay) {
      visibility: hidden;
      opacity: 0;
    }
    body.gmvt-reveal > *:not(#gmvt-overlay) {
      visibility: visible !important;
      opacity: 1 !important;
      transition: opacity 0.7s ease !important;
    }
    #gmvt-overlay {
      position: fixed; inset: 0; z-index: 99998;
      background: #050505; overflow: hidden;
      transition: opacity 0.5s ease;
    }
    #gmvt-overlay.fade-out { opacity: 0; pointer-events: none; }
    #gmvt-overlay video {
      position: absolute; inset: 0;
      width: 100%; height: 100%; object-fit: cover;
    }
    #gmvt-overlay::after {
      content: ''; position: absolute; inset: 0; z-index: 2; pointer-events: none;
      background: repeating-linear-gradient(0deg,transparent,transparent 2px,rgba(0,0,0,0.04) 2px,rgba(0,0,0,0.04) 4px);
    }
    #gmvt-final-img {
      position: absolute; inset: 0; z-index: 3;
      width: 100%; height: 100%; object-fit: contain;
      opacity: 0; transition: opacity 0.2s ease; pointer-events: none;
    }
    #gmvt-final-img.visible { opacity: 1; }
    #gmvt-skip {
      position: absolute; bottom: 2rem; right: 2rem; z-index: 10;
      font-family: 'Share Tech Mono', monospace; font-size: 0.68rem;
      letter-spacing: 0.18em; text-transform: uppercase;
      color: rgba(176,212,176,0.45); border: 1px solid rgba(0,255,136,0.18);
      background: transparent; padding: 0.35rem 0.85rem; cursor: pointer;
      transition: color 0.2s, border-color 0.2s;
    }
    #gmvt-skip:hover { color: #00ff88; border-color: rgba(0,255,136,0.55); }

    /* Flying logo — fixed position, no opacity transition at all */
    #gmvt-fly-logo {
      position: fixed;
      z-index: 99999;
      pointer-events: none;
      transform-origin: top left;
      object-fit: contain;
    }

    /* The real GMVT text brand is hidden the whole time the fly logo exists.
       We do this by targeting it directly via JS, not a body class,
       so there's zero timing gap. */
  `;
  document.head.appendChild(style);

  if (!IS_INDEX || INTRO_DONE) {
    sessionStorage.setItem('gmvt_intro_done', '1');
    return;
  }

  document.documentElement.style.overflow = 'hidden';

  const overlay     = document.createElement('div');
  overlay.id        = 'gmvt-overlay';

  const video       = document.createElement('video');
  video.src         = 'images/intro.mp4';
  video.autoplay    = true;
  video.muted       = true;
  video.playsInline = true;
  video.loop        = false;
  video.preload     = 'auto';

  const finalImg = document.createElement('img');
  finalImg.id    = 'gmvt-final-img';
  finalImg.src   = 'images/final-animation.jpg';
  finalImg.alt   = '';

  const skipBtn       = document.createElement('button');
  skipBtn.id          = 'gmvt-skip';
  skipBtn.textContent = '[ skip intro ]';

  overlay.appendChild(video);
  overlay.appendChild(finalImg);
  overlay.appendChild(skipBtn);

  document.addEventListener('DOMContentLoaded', () => {
    document.body.classList.add('gmvt-loading');
    document.body.appendChild(overlay);
  });

  let fired = false;
  function runTransition() {
    if (fired) return;
    fired = true;
    clearTimeout(fallback);
    video.pause();
    finalImg.classList.add('visible');
    setTimeout(flyToNavbar, 350);
  }

  function flyToNavbar() {
    const vw = window.innerWidth;
    const vh = window.innerHeight;
    const NW = finalImg.naturalWidth  || 1376;
    const NH = finalImg.naturalHeight || 768;
    const imgScale   = Math.min(vw / NW, vh / NH);
    const imgRenderW = NW * imgScale;
    const imgRenderH = NH * imgScale;
    const imgLeft    = (vw - imgRenderW) / 2;
    const imgTop     = (vh - imgRenderH) / 2;

    // Measure navbar brand
    const navBrand = document.querySelector('.navbar-brand');
    let targetX = 32, targetY = 14, targetW = 70, targetH = 28;
    if (navBrand) {
      const prev = navBrand.style.cssText;
      navBrand.style.setProperty('visibility', 'visible', 'important');
      navBrand.style.setProperty('opacity', '0', 'important');
      const r = navBrand.getBoundingClientRect();
      navBrand.style.cssText = prev;
      if (r.width > 0) {
        targetX = r.left;
        targetY = r.top;
        targetW = r.width;
        targetH = r.height;
      }
    }

    // ── Immediately hide the real brand text so it never shows ──
    if (navBrand) {
      navBrand.style.setProperty('opacity', '0', 'important');
      navBrand.style.setProperty('visibility', 'hidden', 'important');
      navBrand.style.setProperty('transition', 'none', 'important');
    }

    // Spawn flying image
    const fly  = document.createElement('img');
    fly.id     = 'gmvt-fly-logo';
    fly.src    = 'images/final-animation.jpg';
    fly.style.cssText = `
      width:  ${imgRenderW}px;
      height: ${imgRenderH}px;
      top:    ${imgTop}px;
      left:   ${imgLeft}px;
    `;
    document.body.appendChild(fly);
    finalImg.style.opacity = '0';

    // Fade overlay, reveal page
    overlay.classList.add('fade-out');
    setTimeout(() => {
      overlay.remove();
      document.body.classList.remove('gmvt-loading');
      document.body.classList.add('gmvt-reveal');
      document.documentElement.style.overflow = '';
    }, 350);

    // Compute transform
    const S = Math.min(targetW / imgRenderW, targetH / imgRenderH);
    const cxAfter = imgLeft + (imgRenderW / 2) * S;
    const cyAfter = imgTop  + (imgRenderH / 2) * S;
    const tx = (targetX + targetW / 2) - cxAfter;
    const ty = (targetY + targetH / 2) - cyAfter;

    fly.getBoundingClientRect(); // force reflow

    // Fly — only transform, NO opacity change
    fly.style.transition = `transform 0.85s cubic-bezier(0.4,0,0.2,1)`;
    fly.style.transform  = `translate(${tx}px,${ty}px) scale(${S})`;

    // After animation completes: show real brand, THEN remove fly in same frame
    setTimeout(() => {
      if (navBrand) {
        // Remove inline styles so CSS takes over naturally
        navBrand.style.removeProperty('opacity');
        navBrand.style.removeProperty('visibility');
        navBrand.style.removeProperty('transition');
      }
      // Remove fly only AFTER brand is guaranteed painted
      requestAnimationFrame(() => {
        requestAnimationFrame(() => {
          fly.remove();
          sessionStorage.setItem('gmvt_intro_done', '1');
        });
      });
    }, 1000);
  }

  function dissolveAndReveal() {
    sessionStorage.setItem('gmvt_intro_done', '1');
    overlay.classList.add('fade-out');
    document.body.classList.remove('gmvt-loading');
    document.body.classList.add('gmvt-reveal');
    document.documentElement.style.overflow = '';
    setTimeout(() => overlay.remove(), 600);
  }

  video.addEventListener('ended', runTransition);
  video.addEventListener('error', dissolveAndReveal);
  skipBtn.addEventListener('click', runTransition);
  const fallback = setTimeout(runTransition, 6500);

})();
