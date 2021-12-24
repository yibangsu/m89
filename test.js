const m89 = require('./m89');
const fs = require('fs');
let crypto = new m89();
crypto.setkey("abcdefghijklmnopqrstuvwxyz");

let count = process.argv.length;
for (let k=2; k<count; k++)
// for (let i=0; i<100; i++)
{
    let i=parseInt(process.argv[k]); 
    let file_name = "enc_file_"+('0000'+i).slice(-4)+".txt";
    console.log("begin write file "+file_name);
    fs.stat(file_name, function(err, stats){
        if (!err)
        {
            fs.unlinkSync(file_name);
        }
    });
    
    for (let j=i*1000000; j<(i+1)*1000000; j++)
    {
        let enc = crypto.enc(j);
        let dec = crypto.dec(enc);
        if (dec !== j)
        {
            console.log("error at "+j);
            fs.writeFileSync(file_name, 
                "error at "+('00000000'+j).slice(-8)+" -> "+('00000000'+enc).slice(-8)+"\r\n", 
                {flag:'a+'},
                function(){});
        }
        else
        {
            fs.writeFileSync(file_name,
                ('00000000'+j).slice(-8)+" -> "+('00000000'+enc).slice(-8)+"\r\n",
                {flag:'a+'},
                function(){});
        }
    }
    console.log("end write file "+file_name);
}
