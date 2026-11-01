# Timer dan IRQ pada WASM32

Adapter WASM mengambil waktu dari host browser atau Node.js. Timer software dapat dipakai oleh diagnostik yang didukung, tetapi browser tidak memberikan guest hardware timer interrupt, GIC, atau IRQ bare-metal. Gunakan `version`, `stats`, dan `irq status` dari shell untuk memeriksa status runtime; output harus menyatakan fasilitas hardware tersebut tidak tersedia.
