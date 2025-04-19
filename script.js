 // Mobile menu toggle
 const menuBtn = document.getElementById('menu-btn');
 const mobileMenu = document.getElementById('mobile-menu');

 menuBtn.addEventListener('click', () => {
     const expanded = menuBtn.getAttribute('aria-expanded') === 'true';
     menuBtn.setAttribute('aria-expanded', !expanded);
     menuBtn.querySelectorAll('svg').forEach(icon => icon.classList.toggle('hidden'));
     mobileMenu.classList.toggle('hidden');
 });

 // Close menu on click outside
 document.addEventListener('click', (e) => {
     if (!menuBtn.contains(e.target) && !mobileMenu.contains(e.target)) {
         mobileMenu.classList.add('hidden');
         menuBtn.setAttribute('aria-expanded', 'false');
         menuBtn.querySelectorAll('svg').forEach(icon => icon.classList.add('hidden'));
         menuBtn.querySelector('svg:first-child').classList.remove('hidden');
     }
 });

 // Close menu on window resize
 window.addEventListener('resize', () => {
     if (window.innerWidth >= 768) {
         mobileMenu.classList.add('hidden');
         menuBtn.setAttribute('aria-expanded', 'false');
         menuBtn.querySelectorAll('svg').forEach(icon => icon.classList.add('hidden'));
         menuBtn.querySelector('svg:first-child').classList.remove('hidden');
     }
 });