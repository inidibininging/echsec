const f = require('fs');
const process = require('process');
let mem = [];
let cs = [];
let p = null;

var ifns = {
  print : function(m) {
    console.log("IFN print");
    console.log(m);
  },
  bind : function(m) {
    console.log("IFN bind");
    console.log(m);
  }
};

function read() {
  p = JSON.parse(f.readFileSync(process.argv[2]).toString());
  if(!p) {
    throw 'p not set';
  }
}

function run() {
  console.log("run");
  const instr = cs.pop();

  console.log("callstack instr popped:");
  console.log(instr);
  
  console.log("looking for instruction index:");
  let mfnidx = p.indexOf(instr);
  console.log(mfnidx);

  if(mfnidx == -1) {
    throw `error. didnt find instr: ${instr ? JSON.stringify(instr) : 'instr is null'}`;
    return;
  }

  let mfn = mfnidx;
  cs.push(p[mfnidx]);
  
  //while(++mfnidx) {
  setTimeout(function() {
	++mfnidx;
    //console.log(s);
	
    if(imfnidx >= p.length) {
      console.log("the end found");
      if(cs.length != 0){
        cs.pop();
      }
      break;
    }
     let s = gs(p[mfnidx]);
    if(!s) {
      throw "error. didnt find scope of instruction.mfnidx:" + mfnidx.toString() + p[mfnidx];
    }



    //endif

    if(p[mfnidx].operation_type == 8) {
      console.log("endif / thatsit");
      console.log(cs);
      continue;
    }

    //if eq
    if(p[mfnidx].operation_type == 6) {
      let lop = null;
      let rop = null;
      
      //--------------
      //alnum => left part is a variable
      if(p[mfnidx].name_ref.type == 16) {
        console.log(p[mfnidx]);
        lop = gv(
          p[mfnidx].name_ref.name,
          s
        )
      }
      //string => left part is a string
      else {
        lop = p[mfnidx].name_ref.name;
      }

      //---------
      //alnum => right part is a variable
      if(p[mfnidx].value_ref.type == 16) {
        rop = gv(
          p[mfnidx].value_ref[0].name,
          s
        )
      }
      //string => right part is a string
      else {
        rop = p[mfnidx].value_ref[0].name;
      }
      console.log("if left op / right op");
      console.log(lop);
      console.log(rop);
      console.log("result:");
      console.log(lop == rop);
      continue;
    }
    //internal fn
    if(p[mfnidx].operation_type == 4) {
      console.log("internal fn found");
      ifns[p[mfnidx].name_ref.name](
        p[mfnidx].value_ref
      );
      continue;
    }

    if(p[mfnidx].operation_type == 2) {
      console.log("found function. end of current fn");
      console.log(p[mfnidx]);
      cs.pop();
      return;
    }

    if(p[mfnidx].operation_type == 1) {
      console.log("setting a variable");
      sv(
        p[mfnidx].name_ref.name,
        s,
        p[mfnidx].value_ref[0].name
      );
      continue;
    }
    //execute 
    if(p[mfnidx].operation_type == 3) {
      console.log("execution found");
      console.log("pushing current instruction to call stack");
      
      //1. add + 1 to mfnidx and look what it is
      //2. - if it is a function => end of the fn => pop twice => really? what about op_Type == 3 chains (multiple op type = 3 after another)
      //2. - if it is the end => end of the fn => pop twice
      console.log(p[mfnidx]);
      if(mfnidx+1 == p.length ||
        p[mfnidx+1].operation_type == 2) {
        //.2 goes wrong? here
        cs.pop();
        break;
      }

      //like pointer to the next instr 
      cs.push(p[mfnidx+1]);

      console.log("pushing function instruction to call stack");

      //the other fn
      cs.push(gf(p[mfnidx].name_ref.name));
      break;
    }
  }, 5000);
}

function gf(n) {
  //console.log('gf');
  const fn = p.filter(i => {
    return i.operation_type == 2 && 
      i.name_ref.name == n;    
  });
  return fn.length == 0 ? null : fn[0];
}

function sv(n, s, v) {
  //console.log('sv');
  const vv = gv(n, s);
  if(vv) {
    vv.v = v;
  }
  else {
    mem.push({
      n: n,
      v: v,
      s: s
    });
  } 
  //console.log(mem);
}

function gs(i) {
  //console.log('gs');
  const iidx = p.indexOf(i);
  for(let x = iidx - 1;x >= 0;x--) {
    if(p[x].operation_type == 2) {
      //console.log("scope found");
      return p[x].name_ref.name;
    }
  }
}

function gv(n, s) {
  //console.log('gv');
  const e = mem.filter(vo => {
    return vo.n == n && vo.s == s;
  });
  if(e.length > 0){
    return e[0];
  }
  return null;
}

read();
cs.push(gf('main'));

while(cs.length != 0){
  console.log("callstack:");
  console.log(cs);
  run();

}
