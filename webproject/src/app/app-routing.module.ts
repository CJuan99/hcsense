import { NgModule } from '@angular/core';

import {RouterModule, Routes} from '@angular/router';

import { LoginComponent } from './auth/login/login.component';
import { SignupComponent } from './auth/signup/signup.component';
import { DashboardComponent } from './dashboard/dashboard.component';
import {MQTTDashboardComponent} from './mqtt_dashboard/mqtt_dashboard.component';

const routes: Routes =[
  {path:'' , component: LoginComponent},
  {path:'login' , component: LoginComponent},
  {path:'signup' , component: SignupComponent},
  {path:'dashboard' , component: DashboardComponent},
  {path:'mqtt' , component: MQTTDashboardComponent}
];


@NgModule({
  imports:[RouterModule.forRoot(routes)],
  exports:[RouterModule]
})

export class AppRoutingModule{}
