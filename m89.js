class M89 {

    #round = 8;
    #max_key_len = this.#round * 4;
    #key_array;
    #base = 0x5F5E100;

    constructor() {
        this.key = null;
    }

    setkey(key) {
        if (typeof key !== 'string')
        {
            key = String(key);
        }

        let fil_len = this.#max_key_len - 1;
        let key_len = key.length > fil_len? fil_len: key.length;
        let pad_len = fil_len - key_len;

        if (fil_len <= key_len) {
            key = key.substring(0, fil_len);
            pad_len = 0;
        }

        let key_array = [];

        key_array.push(...Buffer.from(key));
        if (pad_len > 0) {
            for (let i=0; i<pad_len; i++) {
                key_array.push(pad_len);
            }
        }
        key_array.push(key_len);

        this.#key_array = key_array;
    } 

    #runEnc(a, k)
    {
        /* plus */
        k = k % this.#base;
        a = (a + k) % this.#base;

        /* shift left */
        let b = String(a);
        b = ('00000000'+b).slice(-8);
        b = b.slice(-7)+b.charAt(0);
        a = parseInt(b);

        /* s-box */
        a = (a * 0x3F940AB) % this.#base;

        return a;
    }

    enc(input) {
        if (typeof input !== 'number') {
            let input = parseInt(input);
        }

        for (let i=0; i<this.#round; i++) {
            let enc_k = this.#key_array.slice(i*4, (i+1)*4);
            let k = Math.abs(enc_k[0] << 24) | (enc_k[1] << 16) | (enc_k[2] << 8) | enc_k[3];
            input = this.#runEnc(input, k);
        }

        return input;
    }

    #runDec(a, k)
    {
        /* s-box */
        a = (a * 0x3) % this.#base;

        /* right shift */
        let b = String(a);
        b = ('00000000'+b).slice(-8);
        b = b.charAt(7)+b.slice(0,7);
        a = parseInt(b);

        /* minus */
        k = k % this.#base;
        if (a >= k)
        {
            a = a - k;
        }
        else
        {
            a = a + (this.#base - k);
        }

        return a;
    }

    dec(input) {
        if (typeof input !== 'number') {
            input = parseInt(input);
        }

        for (let i=this.#round-1; i>=0; i--) {
            let dec_k = this.#key_array.slice(i*4, (i+1)*4);
            let k = Math.abs(dec_k[0] << 24) | (dec_k[1] << 16) | (dec_k[2] << 8) | dec_k[3];
            input = this.#runDec(input, k);
        }

        return input;
    }
}

module.exports = M89