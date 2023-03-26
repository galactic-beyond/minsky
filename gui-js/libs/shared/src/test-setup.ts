import 'jest-preset-angular/setup-jest';
//import '../../../apps/minsky-electron/src/app/backend-init';
import * as JSON5 from 'json5';
import {CppClass} from '@minsky/shared';

const restService = require('bindings')('../../node-addons/minskyRESTService.node');

// replace backend with a synchronous call
CppClass.backend=(command: string, ...args: any[])=>{
  var arg='';
  if (args.length>1) {
    arg=JSON5.stringify(args, {quote: '"'});
  } else if (args.length===1) {
    arg=JSON5.stringify(args[0], {quote: '"'});
  }
  console.log(command);
  return JSON5.parse(restService.call(`${command}/$sync`, arg));
};

